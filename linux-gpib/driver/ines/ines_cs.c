/***************************************************************************
                          nec7210/ines_cs.c  -  description
                             -------------------
   support for ines PCMCIA GPIB boards.  Based on Claus Schroeter's
   pcmcia gpib driver, which used the skeleton example (David Hinds probably).

    copyright            : (C) 1999 Axel Dziemba (axel.dziemba@ines.de)
                           (C) 2002 by Frank Mori Hess
    email                : fmhess@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ines.h"

#if defined(GPIB_CONFIG_PCMCIA)

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <asm/io.h>
#include <asm/system.h>

#include <pcmcia/version.h>
#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/ds.h>
#include <pcmcia/cisreg.h>


static int pc_debug = 1;
static char *version =
	"$Id$";

/* Parameters that can be set with 'insmod' */

/* Newer, simpler way of listing specific interrupts */
static int irq_list[4] = { -1 };
MODULE_PARM(irq_list, "1-4i");

/* The old way: bit map of interrupts to choose from */
static int irq_mask = 0xffff;
MODULE_PARM(irq_mask, "i");

static int first_tuple(client_handle_t handle, tuple_t *tuple,
	cisparse_t *parse)
{
	int i;
	i = pcmcia_get_first_tuple(handle, tuple);
	if (i != CS_SUCCESS) return i;
	i = pcmcia_get_tuple_data(handle, tuple);
	if (i != CS_SUCCESS) return i;
	return pcmcia_parse_tuple(handle, tuple, parse);
}
static int next_tuple(client_handle_t handle, tuple_t *tuple,
	cisparse_t *parse)
{
	int i;
	i = pcmcia_get_next_tuple(handle, tuple);
	if (i != CS_SUCCESS) return i;
	i = pcmcia_get_tuple_data(handle, tuple);
	if (i != CS_SUCCESS) return i;
	return pcmcia_parse_tuple(handle, tuple, parse);
}

/*
   The event() function is this driver's Card Services event handler.
   It will be called by Card Services when an appropriate card status
   event is received.  The config() and release() entry points are
   used to configure or release a socket, in response to card insertion
   and ejection events.  They are invoked from the gpib event
   handler.
*/

static void gpib_config(dev_link_t *link);
static void gpib_release(u_long arg);
static int gpib_event(event_t event, int priority,
			  event_callback_args_t *args);

/*
   The attach() and detach() entry points are used to create and destroy
   "instances" of the driver, where each instance represents everything
   needed to manage one actual PCMCIA card.
*/

static dev_link_t *gpib_attach(void);
static void gpib_detach(dev_link_t *);

/*
   You'll also need to prototype all the functions that will actually
   be used to talk to your device.  See 'pcmem_cs' for a good example
   of a fully self-sufficient driver; the other drivers rely more or
   less on other parts of the kernel.
*/

/*
   The dev_info variable is the "key" that is used to match up this
   device driver with appropriate cards, through the card configuration
   database.
*/

static dev_info_t dev_info = "ines_gpib_cs";

/*
   A linked list of "instances" of the gpib device.  Each actual
   PCMCIA card corresponds to one device instance, and is described
   by one dev_link_t structure (defined in ds.h).

   You may not want to use a linked list for this -- for example, the
   memory card driver uses an array of dev_link_t pointers, where minor
   device numbers are used to derive the corresponding array index.
*/

static dev_link_t *dev_list = NULL;

/*
   A dev_link_t structure has fields for most things that are needed
   to keep track of a socket, but there will usually be some device
   specific information that also needs to be kept track of.  The
   'priv' pointer in a dev_link_t structure can be used to point to
   a device-specific private data structure, like this.

   A driver needs to provide a dev_node_t structure for each device
   on a card.  In some cases, there is only one device per card (for
   example, ethernet cards, modems).  In other cases, there may be
   many actual or logical devices (SCSI adapters, memory cards with
   multiple partitions).  The dev_node_t structures need to be kept
   in a linked list starting at the 'dev' field of a dev_link_t
   structure.  We allocate them in the card's private data structure,
   because they generally can't be allocated dynamically.
*/

typedef struct local_info_t {
    dev_node_t	node;
    u_short manfid;
    u_short cardid;
} local_info_t;


/*
    gpib_attach() creates an "instance" of the driver, allocating
    local data structures for one device.  The device is registered
    with Card Services.

    The dev_link structure is initialized, but we don't actually
    configure the card at this point -- we wait until we receive a
    card insertion event.

*/

static dev_link_t *gpib_attach(void)
{
	client_reg_t client_reg;
	dev_link_t *link;
	local_info_t *local;
	int ret;
	int i;

	if (pc_debug)
		printk(KERN_DEBUG "gpib_attach()\n");

	/* Initialize the dev_link_t structure */
	link = kmalloc(sizeof(struct dev_link_t), GFP_KERNEL);
	memset(link, 0, sizeof(struct dev_link_t));

	/* The io structure describes IO port mapping */
	link->io.NumPorts1 =32;
	link->io.Attributes1 = IO_DATA_PATH_WIDTH_8;
	link->io.NumPorts2 = 0;
	link->io.Attributes2 =0;
	link->io.IOAddrLines = 5;

	/* Interrupt setup */
	link->irq.Attributes = IRQ_TYPE_EXCLUSIVE | IRQ_FORCED_PULSE;
	link->irq.IRQInfo1 = IRQ_INFO2_VALID | IRQ_PULSE_ID;
	if(irq_list[0] == -1)
		link->irq.IRQInfo2 = irq_mask;
	else
		for(i = 0; i < 4; i++)
			link->irq.IRQInfo2 |= 1 << irq_list[i];
	printk(KERN_DEBUG "ines_cs: irq_mask=0x%x\n",
		link->irq.IRQInfo2);
	link->irq.Handler = NULL;

	/* General socket configuration */
	link->conf.Attributes = CONF_ENABLE_IRQ;
	link->conf.Vcc = 50;
	link->conf.IntType = INT_MEMORY_AND_IO;
	link->conf.ConfigIndex = 0x30;
	link->conf.Present = PRESENT_OPTION;

	/* Allocate space for private device-specific data */
	local = kmalloc(sizeof(local_info_t), GFP_KERNEL);
	memset(local, 0, sizeof(local_info_t));
	link->priv = local;

	/* Register with Card Services */
	link->next = dev_list;
	dev_list = link;
	client_reg.dev_info = &dev_info;
	client_reg.Attributes = INFO_IO_CLIENT | INFO_CARD_SHARE;
	client_reg.EventMask =
	CS_EVENT_CARD_INSERTION | CS_EVENT_CARD_REMOVAL |
	CS_EVENT_RESET_PHYSICAL | CS_EVENT_CARD_RESET |
	CS_EVENT_PM_SUSPEND | CS_EVENT_PM_RESUME;
	client_reg.event_handler = &gpib_event;
	client_reg.Version = 0x0210;
	client_reg.event_callback_args.client_data = link;
	ret = pcmcia_register_client(&link->handle, &client_reg);
	if (ret != CS_SUCCESS) {
		cs_error(link->handle, RegisterClient, ret);
		gpib_detach(link);
		return NULL;
	}

	return link;
} /* gpib_attach */

/*

    This deletes a driver "instance".  The device is de-registered
    with Card Services.  If it has been released, all local data
    structures are freed.  Otherwise, the structures will be freed
    when the device is released.

*/

static void gpib_detach(dev_link_t *link)
{
	dev_link_t **linkp;

	if (pc_debug)
		printk(KERN_DEBUG "gpib_detach(0x%p)\n", link);

	/* Locate device structure */
	for (linkp = &dev_list; *linkp; linkp = &(*linkp)->next)
	if (*linkp == link) break;
	if (*linkp == NULL) return;

	/*
		If the device is currently configured and active, we won't
		actually delete it yet.  Instead, it is marked so that when
		the release() function is called, that will trigger a proper
		detach().
	*/
	if (link->state & DEV_CONFIG) {
		printk(KERN_DEBUG "ines_cs: detach postponed, '%s' "
			"still locked\n", link->dev->dev_name);
		link->state |= DEV_STALE_LINK;
		return;
	}

    /* Break the link with Card Services */
	if (link->handle)
		pcmcia_deregister_client(link->handle);

	/* Unlink device structure, free pieces */
	*linkp = link->next;
	if (link->priv) {
		kfree(link->priv);
	}

} /* gpib_detach */

/*

    gpib_config() is scheduled to run after a CARD_INSERTION event
    is received, to configure the PCMCIA socket, and to make the
    ethernet device available to the system.

*/
static void gpib_config(dev_link_t *link)
{
	tuple_t tuple;
	cisparse_t parse;
	local_info_t *dev;
	int i;
	u_char buf[64];
	win_req_t req;
	memreq_t mem;
	unsigned long virt;

	dev = link->priv;

	if (pc_debug)
		printk(KERN_DEBUG "gpib_config(0x%p)\n", link);

	/*
		This reads the card's CONFIG tuple to find its configuration
		registers.
	*/
	do {
		tuple.DesiredTuple = CISTPL_CONFIG;
		i = pcmcia_get_first_tuple(link->handle, &tuple);
		if (i != CS_SUCCESS) break;
		tuple.TupleData = buf;
		tuple.TupleDataMax = 64;
		tuple.TupleOffset = 0;
		i = pcmcia_get_tuple_data(link->handle, &tuple);
		if (i != CS_SUCCESS) break;
		i = pcmcia_parse_tuple(link->handle, &tuple, &parse);
		if (i != CS_SUCCESS) break;
		link->conf.ConfigBase = parse.config.base;
		link->conf.Present = parse.config.rmask[0];
    } while (0);
	if (i != CS_SUCCESS) {
		cs_error(link->handle, ParseTuple, i);
		link->state &= ~DEV_CONFIG_PENDING;
		return;
	}

	/* Configure card */
	link->state |= DEV_CONFIG;
	do {
		/*
		 * try to get manufacturer and card  ID
		 */
		tuple.DesiredTuple = CISTPL_MANFID;
		tuple.Attributes = TUPLE_RETURN_COMMON;
		if( first_tuple(link->handle,&tuple,&parse) == CS_SUCCESS ) {
			dev->manfid = parse.manfid.manf;
			dev->cardid = parse.manfid.card;
			printk(KERN_DEBUG "ines_cs: manufacturer: 0x%x card: 0x%x\n",
			dev->manfid, dev->cardid);
		}
		/* try to get board information from CIS */

		tuple.DesiredTuple = CISTPL_CFTABLE_ENTRY;
		tuple.Attributes = 0;
		if( first_tuple(link->handle,&tuple,&parse) == CS_SUCCESS ) {
			while(1) {
				if( parse.cftable_entry.io.nwin > 0) {
					link->io.BasePort1 = parse.cftable_entry.io.win[0].base;
					link->io.NumPorts1 = 32;
					link->io.BasePort2 = 0;
					link->io.NumPorts2 = 0;
					i = pcmcia_request_io(link->handle, &link->io);
					if (i == CS_SUCCESS) {
					printk( KERN_DEBUG "ines_cs: base=0x%x len=%d registered\n",
						link->io.BasePort1, link->io.NumPorts1 );
					break;
					}
				}
				if ( next_tuple(link->handle,&tuple,&parse) != CS_SUCCESS ) break;
			}

			if (i != CS_SUCCESS) {
				cs_error(link->handle, RequestIO, i);
			}
		} else {
			printk("ines_cs: can't get card information\n");
		}

		link->conf.Status = CCSR_IOIS8;

		/*  for the ines card we have to setup the configuration registers in
			attribute memory here
		*/
		req.Attributes=WIN_MEMORY_TYPE_AM | WIN_DATA_WIDTH_8 | WIN_ENABLE;
		req.Base=0;
		req.Size=0x1000;
		req.AccessSpeed=250;
		i= pcmcia_request_window(&link->handle, &req, &link->win);
		if (i != CS_SUCCESS) {
			cs_error(link->handle, RequestWindow, i);
			break;
		}
		mem.CardOffset=0;
		mem.Page=0;
		i= pcmcia_map_mem_page(link->win, &mem);
		if (i != CS_SUCCESS) {
			cs_error(link->handle, MapMemPage, i);
			break;
		}
		virt = ( unsigned long ) ioremap( req.Base, req.Size );
		writeb( ( link->io.BasePort1 >> 2 ) & 0xff, virt + 0xf0 ); // IOWindow base
		iounmap( ( void* ) virt );

	} while (0);

	/*
	Now allocate an interrupt line.
	*/
	if (link->conf.Attributes & CONF_ENABLE_IRQ)
	{
		i = pcmcia_request_irq(link->handle, &link->irq);
		if (i != CS_SUCCESS) {
			cs_error(link->handle, RequestIRQ, i);
		}
		printk(KERN_DEBUG "ines_cs: IRQ_Line=%d\n",link->irq.AssignedIRQ);
	}

	/*
	This actually configures the PCMCIA socket -- setting up
	the I/O windows and the interrupt mapping.
	*/
	i = pcmcia_request_configuration(link->handle, &link->conf);
	if (i != CS_SUCCESS) {
		cs_error(link->handle, RequestConfiguration, i);
	}

	/* At this point, the dev_node_t structure(s) should be
	initialized and arranged in a linked list at link->dev. */
	sprintf(dev->node.dev_name, "ines_cs");
	dev->node.major = 0;
	dev->node.minor = 0;
	link->dev = &dev->node;

	link->state &= ~DEV_CONFIG_PENDING;
	/* If any step failed, release any partially configured state */
	if (i != CS_SUCCESS) {
		gpib_release((u_long)link);
		return;
	}

	printk(KERN_DEBUG "gpib device loaded\n");
} /* gpib_config */

/*

    After a card is removed, gpib_release() will unregister the net
    device, and release the PCMCIA configuration.  If the device is
    still open, this will be postponed until it is closed.

*/

static void gpib_release(u_long arg)
{
    dev_link_t *link = (dev_link_t *)arg;

    if (pc_debug)
	printk(KERN_DEBUG "gpib_release(0x%p)\n", link);

    /*
       If the device is currently in use, we won't release until it
       is actually closed.
    */
    if (link->open) {
	if (pc_debug)
	    printk(KERN_DEBUG "ines_cs: release postponed, '%s' "
		   "still open\n", link->dev->dev_name);
       link->state |= DEV_STALE_CONFIG;
	return;
    }

    /* Unlink the device chain */
    link->dev = NULL;
    
    /* Don't bother checking to see if these succeed or not */
    pcmcia_release_window(link->win);
    pcmcia_release_configuration(link->handle);
    pcmcia_release_io(link->handle, &link->io);
    pcmcia_release_irq(link->handle, &link->irq);
    link->state &= ~DEV_CONFIG;
    
    if (link->state & DEV_STALE_LINK)
	gpib_detach(link);
    
} /* gpib_release */

/*

    The card status event handler.  Mostly, this schedules other
    stuff to run after an event is received.  A CARD_REMOVAL event
    also sets some flags to discourage the net drivers from trying
    to talk to the card any more.

    When a CARD_REMOVAL event is received, we immediately set a flag
    to block future accesses to this device.  All the functions that
    actually access the device should check this flag to make sure
    the card is still present.

*/

static int gpib_event(event_t event, int priority,
			  event_callback_args_t *args)
{
    dev_link_t *link = args->client_data;

    if (pc_debug)
	printk(KERN_DEBUG "gpib_event()\n");

    switch (event) {
    case CS_EVENT_REGISTRATION_COMPLETE:
	if (pc_debug)
	    printk(KERN_DEBUG "ines_cs: registration complete\n");
	break;
    case CS_EVENT_CARD_REMOVAL:
	link->state &= ~DEV_PRESENT;
	if (link->state & DEV_CONFIG) {
		gpib_release((u_long)link);
	}
	break;
    case CS_EVENT_CARD_INSERTION:
	link->state |= DEV_PRESENT | DEV_CONFIG_PENDING;
	gpib_config(link);
	break;
    case CS_EVENT_PM_SUSPEND:
	link->state |= DEV_SUSPEND;
	/* Fall through... */
    case CS_EVENT_RESET_PHYSICAL:
	if (link->state & DEV_CONFIG)
	    pcmcia_release_configuration(link->handle);
	break;
    case CS_EVENT_PM_RESUME:
	link->state &= ~DEV_SUSPEND;
	/* Fall through... */
    case CS_EVENT_CARD_RESET:
	if (link->state & DEV_CONFIG)
	    pcmcia_request_configuration(link->handle, &link->conf);
	break;
    }
    return 0;
} /* gpib_event */

static struct pcmcia_driver ines_gpib_cs_driver =
{
	.attach = gpib_attach,
	.detach = gpib_detach,
	.owner = THIS_MODULE,
	.drv = {
		.name = "ines_gpib_cs",
	},
};


int ines_pcmcia_init_module(void)
{
	servinfo_t serv;
	if (pc_debug)
		printk(KERN_INFO "%s\n", version);
	pcmcia_get_card_services_info(&serv);
	if (serv.Revision != CS_RELEASE_CODE)
	{
		printk(KERN_NOTICE "gpib: Card Services release "
			"does not match!\n");
		return -1;
	}
	pcmcia_register_driver(&ines_gpib_cs_driver);
	return 0;
}

void ines_pcmcia_cleanup_module(void)
{
	if (pc_debug)
		printk(KERN_DEBUG "ines_cs: unloading\n");
	pcmcia_unregister_driver(&ines_gpib_cs_driver);
	while (dev_list != NULL) 
	{
		if (dev_list->state & DEV_CONFIG)
		    gpib_release((u_long)dev_list);
		gpib_detach(dev_list);
    }
}

int ines_pcmcia_attach(gpib_board_t *board);
int ines_pcmcia_accel_attach(gpib_board_t *board);
void ines_pcmcia_detach(gpib_board_t *board);

gpib_interface_t ines_pcmcia_unaccel_interface =
{
	name: "ines_pcmcia_unaccel",
	attach: ines_pcmcia_attach,
	detach: ines_pcmcia_detach,
	read: ines_read,
	write: ines_write,
	command: ines_command,
	take_control: ines_take_control,
	go_to_standby: ines_go_to_standby,
	request_system_control: ines_request_system_control,
	interface_clear: ines_interface_clear,
	remote_enable: ines_remote_enable,
	enable_eos: ines_enable_eos,
	disable_eos: ines_disable_eos,
	parallel_poll: ines_parallel_poll,
	parallel_poll_configure: ines_parallel_poll_configure,
	parallel_poll_response: ines_parallel_poll_response,
	line_status: ines_line_status,
	update_status: ines_update_status,
	primary_address: ines_primary_address,
	secondary_address: ines_secondary_address,
	serial_poll_response: ines_serial_poll_response,
	serial_poll_status: ines_serial_poll_status,
	t1_delay: ines_t1_delay,
	return_to_local: ines_return_to_local,
};

gpib_interface_t ines_pcmcia_accel_interface =
{
	name: "ines_pcmcia_accel",
	attach: ines_pcmcia_accel_attach,
	detach: ines_pcmcia_detach,
	read: ines_accel_read,
	write: ines_accel_write,
	command: ines_command,
	take_control: ines_take_control,
	go_to_standby: ines_go_to_standby,
	request_system_control: ines_request_system_control,
	interface_clear: ines_interface_clear,
	remote_enable: ines_remote_enable,
	enable_eos: ines_enable_eos,
	disable_eos: ines_disable_eos,
	parallel_poll: ines_parallel_poll,
	parallel_poll_configure: ines_parallel_poll_configure,
	parallel_poll_response: ines_parallel_poll_response,
	line_status: ines_line_status,
	update_status: ines_update_status,
	primary_address: ines_primary_address,
	secondary_address: ines_secondary_address,
	serial_poll_response: ines_serial_poll_response,
	serial_poll_status: ines_serial_poll_status,
	t1_delay: ines_t1_delay,
	return_to_local: ines_return_to_local,
};

gpib_interface_t ines_pcmcia_interface =
{
	name: "ines_pcmcia",
	attach: ines_pcmcia_accel_attach,
	detach: ines_pcmcia_detach,
	read: ines_accel_read,
	write: ines_accel_write,
	command: ines_command,
	take_control: ines_take_control,
	go_to_standby: ines_go_to_standby,
	request_system_control: ines_request_system_control,
	interface_clear: ines_interface_clear,
	remote_enable: ines_remote_enable,
	enable_eos: ines_enable_eos,
	disable_eos: ines_disable_eos,
	parallel_poll: ines_parallel_poll,
	parallel_poll_configure: ines_parallel_poll_configure,
	parallel_poll_response: ines_parallel_poll_response,
	line_status: ines_line_status,
	update_status: ines_update_status,
	primary_address: ines_primary_address,
	secondary_address: ines_secondary_address,
	serial_poll_response: ines_serial_poll_response,
	serial_poll_status: ines_serial_poll_status,
	t1_delay: ines_t1_delay,
	return_to_local: ines_return_to_local,
};

int ines_common_pcmcia_attach( gpib_board_t *board )
{
	ines_private_t *ines_priv;
	nec7210_private_t *nec_priv;
	int retval;

	if(dev_list == NULL)
	{
		printk("no ines pcmcia cards found\n");
		return -1;
	}

	retval = ines_generic_attach(board);
	if(retval) return retval;

	ines_priv = board->private_data;
	nec_priv = &ines_priv->nec7210_priv;

	nec_priv->iobase = dev_list->io.BasePort1;

	nec7210_board_reset( nec_priv, board );

	if(request_irq(dev_list->irq.AssignedIRQ, ines_interrupt, 0, "pcmcia-gpib", board))
	{
		printk("gpib: can't request IRQ %d\n", dev_list->irq.AssignedIRQ);
		return -1;
	}
	ines_priv->irq = dev_list->irq.AssignedIRQ;

	return 0;
}

int ines_pcmcia_attach( gpib_board_t *board )
{
	ines_private_t *ines_priv;
	int retval;

	retval = ines_common_pcmcia_attach( board );
	if( retval < 0 ) return retval;

	ines_priv = board->private_data;
	ines_online( ines_priv, board, 0 );

	return 0;
}

int ines_pcmcia_accel_attach( gpib_board_t *board )
{
	ines_private_t *ines_priv;
	int retval;

	retval = ines_common_pcmcia_attach( board );
	if( retval < 0 ) return retval;

	ines_priv = board->private_data;
	ines_online( ines_priv, board, 1 );

	return 0;
}

void ines_pcmcia_detach(gpib_board_t *board)
{
	ines_private_t *priv = board->private_data;

	if(priv && priv->irq)
		free_irq(priv->irq, board);
	ines_free_private(board);
}

#endif /* CONFIG_PCMCIA */
