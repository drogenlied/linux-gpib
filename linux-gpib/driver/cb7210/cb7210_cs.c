/***************************************************************************
                              nec7210/cb7210_cs.c
                             -------------------
    Support for computer boards pcmcia-gpib card

    Based on gpib PCMCIA client driver written by Claus Schroeter
    (clausi@chemie.fu-berlin.de), which was adapted from the
    pcmcia skeleton example (presumably David Hinds)

    begin                : Jan 2002
    copyright            : (C) 2002 by Frank Mori Hess
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

#include "cb7210.h"

#if defined(GPIB_CONFIG_PCMCIA)

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <asm/io.h>
#include <asm/system.h>

#include <pcmcia/version.h>
#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/ds.h>




/*
   All the PCMCIA modules use PCMCIA_DEBUG to control debugging.  If
   you do not define PCMCIA_DEBUG at all, all the debug code will be
   left out.  If you compile with PCMCIA_DEBUG=0, the debug code will
   be present but disabled -- but it can then be enabled for specific
   modules at load time with a 'pc_debug=#' option to insmod.
*/

#define PCMCIA_DEBUG 1

#ifdef PCMCIA_DEBUG
static int pc_debug = PCMCIA_DEBUG;
static char *version =
"cb7210_cs.c 0.11";
#endif

/*====================================================================*/

/* Parameters that can be set with 'insmod' */

/* Bit map of interrupts to choose from */
/* This means pick from 15, 14, 12, 11, 10, 9, 7, 5, 4, and 3 */
static u_long irq_mask = 0xdeb8;

/*====================================================================*/


static int get_tuple(int fn, client_handle_t handle, tuple_t *tuple,
		     cisparse_t *parse)
{
    int i;
    i = CardServices(fn, handle, tuple);
    if (i != CS_SUCCESS) return i;
    i = CardServices(GetTupleData, handle, tuple);
    if (i != CS_SUCCESS) return i;
    return CardServices(ParseTuple, handle, tuple, parse);
}

#define first_tuple(a, b, c) get_tuple(GetFirstTuple, a, b, c)
#define next_tuple(a, b, c) get_tuple(GetNextTuple, a, b, c)

/*====================================================================*/

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

static dev_info_t dev_info = "cb_gpib_cs";

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

/*====================================================================*/

static void cs_error(client_handle_t handle, int func, int ret)
{
    error_info_t err = { func, ret };
    CardServices(ReportError, handle, &err);
}

/*======================================================================

    gpib_attach() creates an "instance" of the driver, allocating
    local data structures for one device.  The device is registered
    with Card Services.

    The dev_link structure is initialized, but we don't actually
    configure the card at this point -- we wait until we receive a
    card insertion event.

======================================================================*/

static dev_link_t *gpib_attach(void)
{
    client_reg_t client_reg;
    dev_link_t *link;
    local_info_t *local;
    int ret;

#ifdef PCMCIA_DEBUG
    if (pc_debug)
	printk(KERN_DEBUG "gpib_attach()\n");
#endif

    /* Initialize the dev_link_t structure */
    link = kmalloc(sizeof(struct dev_link_t), GFP_KERNEL);
    memset(link, 0, sizeof(struct dev_link_t));
    link->release.function = &gpib_release;
    link->release.data = (u_long)link;

    /* The io structure describes IO port mapping */
    link->io.NumPorts1 = 16;
    link->io.Attributes1 = IO_DATA_PATH_WIDTH_8;
    link->io.NumPorts2 = 16;
    link->io.Attributes2 = IO_DATA_PATH_WIDTH_16;
    link->io.IOAddrLines = 5;

    /* Interrupt setup */
    link->irq.Attributes = IRQ_TYPE_EXCLUSIVE;
    link->irq.IRQInfo1 = IRQ_INFO2_VALID|IRQ_LEVEL_ID;
    link->irq.IRQInfo2 = irq_mask;
    link->irq.Handler = NULL;
    link->irq.Instance = NULL;
    
    /* General socket configuration */
    link->conf.Attributes = CONF_ENABLE_IRQ;
    link->conf.Vcc = 50;
    link->conf.IntType = INT_MEMORY_AND_IO;
    link->conf.ConfigIndex = 1;
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
    ret = CardServices(RegisterClient, &link->handle, &client_reg);
    if (ret != 0) {
	cs_error(link->handle, RegisterClient, ret);
	gpib_detach(link);
	return NULL;
    }

    return link;
} /* gpib_attach */

/*======================================================================

    This deletes a driver "instance".  The device is de-registered
    with Card Services.  If it has been released, all local data
    structures are freed.  Otherwise, the structures will be freed
    when the device is released.

======================================================================*/

static void gpib_detach(dev_link_t *link)
{
    dev_link_t **linkp;

#ifdef PCMCIA_DEBUG
    if (pc_debug)
	printk(KERN_DEBUG "gpib_detach(0x%p)\n", link);
#endif
    
    /* Locate device structure */
    for (linkp = &dev_list; *linkp; linkp = &(*linkp)->next)
	if (*linkp == link) break;
    if (*linkp == NULL)
	return;

    /*
       If the device is currently configured and active, we won't
       actually delete it yet.  Instead, it is marked so that when
       the release() function is called, that will trigger a proper
       detach().
    */
    if (link->state & DEV_CONFIG) {
#ifdef PCMCIA_DEBUG
	printk(KERN_DEBUG "gpib_cs: detach postponed, '%s' "
	       "still locked\n", link->dev->dev_name);
#endif
	link->state |= DEV_STALE_LINK;
	return;
    }

    /* Break the link with Card Services */
    if (link->handle)
	CardServices(DeregisterClient, link->handle);
    
    /* Unlink device structure, free pieces */
    *linkp = link->next;
    if (link->priv) {
	kfree(link->priv);
    }
    
} /* gpib_detach */

/*======================================================================

    gpib_config() is scheduled to run after a CARD_INSERTION event
    is received, to configure the PCMCIA socket, and to make the
    ethernet device available to the system.
    
======================================================================*/
/*@*/
static void gpib_config(dev_link_t *link)
{
    client_handle_t handle;
    tuple_t tuple;
    cisparse_t parse;
    local_info_t *dev;
    int i;
    u_char buf[64];

    handle = link->handle;
    dev = link->priv;

#ifdef PCMCIA_DEBUG
    if (pc_debug)
	printk(KERN_DEBUG "gpib_config(0x%p)\n", link);
#endif

    /*
       This reads the card's CONFIG tuple to find its configuration
       registers.
    */
    do {
	tuple.DesiredTuple = CISTPL_CONFIG;
	i = CardServices(GetFirstTuple, handle, &tuple);
	if (i != CS_SUCCESS) break;
	tuple.TupleData = buf;
	tuple.TupleDataMax = 64;
	tuple.TupleOffset = 0;
	i = CardServices(GetTupleData, handle, &tuple);
	if (i != CS_SUCCESS) break;
	i = CardServices(ParseTuple, handle, &tuple, &parse);
	if (i != CS_SUCCESS) break;
	link->conf.ConfigBase = parse.config.base;
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
        tuple.Attributes   = TUPLE_RETURN_COMMON;
        if( first_tuple(handle,&tuple,&parse) == CS_SUCCESS ) {
	   dev->manfid = parse.manfid.manf;
	   dev->cardid = parse.manfid.card; 
#ifdef PCMCIA_DEBUG	   
	  printk(KERN_DEBUG "gpib_cs: manufacturer: 0x%x card: 0x%x\n",
		 dev->manfid,dev->cardid);
#endif       
	}
        /* try to get board information from CIS */
       
         tuple.DesiredTuple = CISTPL_CFTABLE_ENTRY;
         tuple.Attributes = 0;
         if( first_tuple(handle,&tuple,&parse) == CS_SUCCESS ) {
	    while(1) {
	      /*if this tuple has an IRQ info, keep it for later use */ 
	      if( parse.cftable_entry.irq.IRQInfo1 & IRQ_INFO2_VALID ) {
		printk(KERN_DEBUG "gpib_cs: irqmask=0x%x\n",
		       parse.cftable_entry.irq.IRQInfo2 );
		link->irq.IRQInfo2 = parse.cftable_entry.irq.IRQInfo2;
	      }

	      if( parse.cftable_entry.io.nwin > 0) {   
	         link->io.BasePort1 = parse.cftable_entry.io.win[0].base;
	         link->io.NumPorts1 = parse.cftable_entry.io.win[0].len;
	         link->io.BasePort2 = 0;
	         link->io.NumPorts2 = 0;
	         i = CardServices(RequestIO, link->handle, &link->io);
	         if (i == CS_SUCCESS) {
		     printk( KERN_DEBUG "gpib_cs: base=0x%x len=%d registered\n",
		       parse.cftable_entry.io.win[0].base,
		       parse.cftable_entry.io.win[0].len
		       );
		     break;
	         }
	      }
	      if ( next_tuple(handle,&tuple,&parse) != CS_SUCCESS ) break;

	    }

	  if (i != CS_SUCCESS) {
	      cs_error(link->handle, RequestIO, i);
	  }
	 } else {
	    printk("gpib_cs: can't get card information\n");
	 }

	/*
	   Now allocate an interrupt line.  Note that this does not
	   actually assign a handler to the interrupt.
	*/
	i = CardServices(RequestIRQ, link->handle, &link->irq);
	if (i != CS_SUCCESS) {
	    cs_error(link->handle, RequestIRQ, i);
	    break;
	}
        printk(KERN_DEBUG "gpib_cs: IRQ_Line=%d\n",link->irq.AssignedIRQ);


	/*
	   This actually configures the PCMCIA socket -- setting up
	   the I/O windows and the interrupt mapping.
	*/
	i = CardServices(RequestConfiguration, link->handle, &link->conf);
	if (i != CS_SUCCESS) {
	    cs_error(link->handle, RequestConfiguration, i);
	    break;
	}
    } while (0);

    /* At this point, the dev_node_t structure(s) should be
       initialized and arranged in a linked list at link->dev. */
    sprintf(dev->node.dev_name, "cb_gpib_cs");
    dev->node.major = 0;
    dev->node.minor = 0;
    link->dev = &dev->node;
    
    link->state &= ~DEV_CONFIG_PENDING;
    /* If any step failed, release any partially configured state */
    if (i != 0) {
	gpib_release((u_long)link);
	return;
    }

    printk(KERN_DEBUG "gpib device loaded\n");
} /* gpib_config */

/*======================================================================

    After a card is removed, gpib_release() will unregister the net
    device, and release the PCMCIA configuration.  If the device is
    still open, this will be postponed until it is closed.
    
======================================================================*/

static void gpib_release(u_long arg)
{
    dev_link_t *link = (dev_link_t *)arg;
//    local_info_t *local = link->priv;

#ifdef PCMCIA_DEBUG
    if (pc_debug)
	printk(KERN_DEBUG "gpib_release(0x%p)\n", link);
#endif

    /*
       If the device is currently in use, we won't release until it
       is actually closed.
    */
    if (link->open) {
#ifdef PCMCIA_DEBUG
	if (pc_debug)
	    printk(KERN_DEBUG "gpib_cs: release postponed, '%s' "
		   "still open\n", link->dev->dev_name);
#endif
       link->state |= DEV_STALE_CONFIG;
	return;
    }

    /* Unlink the device chain */
    link->dev = NULL;
    
    /* Don't bother checking to see if these succeed or not */
    CardServices(ReleaseWindow, link->win);
    CardServices(ReleaseConfiguration, link->handle);
    CardServices(ReleaseIO, link->handle, &link->io);
    CardServices(ReleaseIRQ, link->handle, &link->irq);
    link->state &= ~DEV_CONFIG;

    if (link->state & DEV_STALE_LINK)
	gpib_detach(link);
    
} /* gpib_release */

/*======================================================================

    The card status event handler.  Mostly, this schedules other
    stuff to run after an event is received.  A CARD_REMOVAL event
    also sets some flags to discourage the net drivers from trying
    to talk to the card any more.

    When a CARD_REMOVAL event is received, we immediately set a flag
    to block future accesses to this device.  All the functions that
    actually access the device should check this flag to make sure
    the card is still present.
    
======================================================================*/

static int gpib_event(event_t event, int priority,
			  event_callback_args_t *args)
{
    dev_link_t *link = args->client_data;

#ifdef PCMCIA_DEBUG
    if (pc_debug)
	printk(KERN_DEBUG "gpib_event()\n");
#endif
    
    switch (event) {
#ifdef PCMCIA_DEBUG
    case CS_EVENT_REGISTRATION_COMPLETE:
	if (pc_debug)
	    printk(KERN_DEBUG "gpib_cs: registration complete\n");
	break;
#endif
    case CS_EVENT_CARD_REMOVAL:
	link->state &= ~DEV_PRESENT;
	if (link->state & DEV_CONFIG) {
//			((local_info_t *)link->priv)->stop = 1;
			mod_timer(&link->release, jiffies + HZ/20);
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
	    CardServices(ReleaseConfiguration, link->handle);
	break;
    case CS_EVENT_PM_RESUME:
	link->state &= ~DEV_SUSPEND;
	/* Fall through... */
    case CS_EVENT_CARD_RESET:
	if (link->state & DEV_CONFIG)
	    CardServices(RequestConfiguration, link->handle, &link->conf);
	break;
    }
    return 0;
} /* gpib_event */

/*====================================================================*/

int cb_pcmcia_init_module(void)
{
    servinfo_t serv;
#ifdef PCMCIA_DEBUG
    if (pc_debug)
	printk(KERN_INFO "%s\n", version);
#endif
    CardServices(GetCardServicesInfo, &serv);
    if (serv.Revision != CS_RELEASE_CODE) {
	printk(KERN_NOTICE "gpib: Card Services release "
	       "does not match!\n");
	return -1;
    }
    register_pcmcia_driver(&dev_info, &gpib_attach, &gpib_detach);
    return 0;
}

void cb_pcmcia_cleanup_module(void)
{
#ifdef PCMCIA_DEBUG
    if (pc_debug)
	printk(KERN_DEBUG "gpib_cs: unloading\n");
#endif
    unregister_pcmcia_driver(&dev_info);
    while (dev_list != NULL) {
	if (dev_list->state & DEV_CONFIG)
	    gpib_release((u_long)dev_list);
	gpib_detach(dev_list);
    }
}

int cb_pcmcia_attach(gpib_board_t *board);
void cb_pcmcia_detach(gpib_board_t *board);

gpib_interface_t cb_pcmcia_interface =
{
	name: "cbi_pcmcia",
	attach: cb_pcmcia_attach,
	detach: cb_pcmcia_detach,
	read: cb7210_read,
	write: cb7210_write,
	command: cb7210_command,
	take_control: cb7210_take_control,
	go_to_standby: cb7210_go_to_standby,
	interface_clear: cb7210_interface_clear,
	remote_enable: cb7210_remote_enable,
	enable_eos: cb7210_enable_eos,
	disable_eos: cb7210_disable_eos,
	parallel_poll: cb7210_parallel_poll,
	line_status: NULL,	//XXX
	update_status: cb7210_update_status,
	primary_address: cb7210_primary_address,
	secondary_address: cb7210_secondary_address,
	serial_poll_response: cb7210_serial_poll_response,
};

int cb_pcmcia_attach(gpib_board_t *board)
{
	cb7210_private_t *cb_priv;
	nec7210_private_t *nec_priv;
	int isr_flags = 0;
	int retval;

	if(dev_list == NULL)
	{
		printk("no cb pcmcia cards found\n");
		return -1;
	}

	retval = cb7210_generic_attach(board);
	if(retval) return retval;

	cb_priv = board->private_data;
	nec_priv = &cb_priv->nec7210_priv;

	nec_priv->iobase = dev_list->io.BasePort1;

	/* CBI 4882 reset */
	write_byte(nec_priv, HS_RESET7210, HS_INT_LEVEL);
	write_byte(nec_priv, 0, HS_INT_LEVEL);
	write_byte(nec_priv, 0, HS_MODE); /* disable system control */

	if(request_irq(dev_list->irq.AssignedIRQ, cb7210_interrupt, isr_flags, "pcmcia-gpib", board))
	{
		printk("gpib: can't request IRQ %d\n", dev_list->irq.AssignedIRQ);
		return -1;
	}
	cb_priv->irq = dev_list->irq.AssignedIRQ;

	cb7210_init(cb_priv);

	return 0;
}

void cb_pcmcia_detach(gpib_board_t *board)
{
	cb7210_private_t *priv = board->private_data;

	if(priv && priv->irq)
		free_irq(priv->irq, board);
	cb7210_free_private(board);
}

#endif /* CONFIG_PCMCIA */
