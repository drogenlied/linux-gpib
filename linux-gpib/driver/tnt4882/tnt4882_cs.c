/*======================================================================

	Support for National Instruments PCMCIA GPIB cards

	Based on dummy PCMCIA client driver by David A. Hinds

	Copyright (C) 1999 David A. Hinds
	Copyright (C) 2002 Frank Mori Hess

======================================================================*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "tnt4882.h"

#if defined( GPIB_CONFIG_PCMCIA )

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/ioport.h>
#include <asm/io.h>
#include <asm/system.h>

#include <pcmcia/version.h>
#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/cisreg.h>
#include <pcmcia/ds.h>
#include <pcmcia/bus_ops.h>

/*====================================================================*/

/* Module parameters */

#define INT_MODULE_PARM(n, v) static int n = v; MODULE_PARM(n, "i")

/* Newer, simpler way of listing specific interrupts */
static int irq_list[4] = { -1 };
MODULE_PARM(irq_list, "1-4i");

/* Release IO ports after configuration? */
INT_MODULE_PARM(free_ports, 0);

/* The old way: bit map of interrupts to choose from */
/* This means pick from 15, 14, 12, 11, 10, 9, 7, 5, 4, and 3 */
INT_MODULE_PARM(irq_mask, 0xdeb8);

/*
   All the PCMCIA modules use PCMCIA_DEBUG to control debugging.  If
   you do not define PCMCIA_DEBUG at all, all the debug code will be
   left out.  If you compile with PCMCIA_DEBUG=0, the debug code will
   be present but disabled -- but it can then be enabled for specific
   modules at load time with a 'pc_debug=#' option to insmod.
*/
#ifdef PCMCIA_DEBUG
INT_MODULE_PARM(pc_debug, PCMCIA_DEBUG);
#define DEBUG(n, args...) if (pc_debug>(n)) printk(KERN_DEBUG args)
static char *version =
"tnt4882_cs.c (Frank Mori Hess)";
#else
#define DEBUG(n, args...)
#endif

/*====================================================================*/

/*
   The event() function is this driver's Card Services event handler.
   It will be called by Card Services when an appropriate card status
   event is received.  The config() and release() entry points are
   used to configure or release a socket, in response to card
   insertion and ejection events.  They are invoked from the dummy
   event handler. 
*/

static void ni_gpib_config(dev_link_t *link);
static void ni_gpib_release(u_long arg);
static int ni_gpib_event(event_t event, int priority,
		       event_callback_args_t *args);

/*
   The attach() and detach() entry points are used to create and destroy
   "instances" of the driver, where each instance represents everything
   needed to manage one actual PCMCIA card.
*/

static dev_link_t *ni_gpib_attach(void);
static void ni_gpib_detach(dev_link_t *);

/*
   You'll also need to prototype all the functions that will actually
   be used to talk to your device.  See 'memory_cs' for a good example
   of a fully self-sufficient driver; the other drivers rely more or
   less on other parts of the kernel.
*/

/*
   The dev_info variable is the "key" that is used to match up this
   device driver with appropriate cards, through the card configuration
   database.
*/

static dev_info_t dev_info = "ni_gpib_cs";

/*
   A linked list of "instances" of the dummy device.  Each actual
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

   To simplify the data structure handling, we actually include the
   dev_link_t structure in the device's private data structure.

   A driver needs to provide a dev_node_t structure for each device
   on a card.  In some cases, there is only one device per card (for
   example, ethernet cards, modems).  In other cases, there may be
   many actual or logical devices (SCSI adapters, memory cards with
   multiple partitions).  The dev_node_t structures need to be kept
   in a linked list starting at the 'dev' field of a dev_link_t
   structure.  We allocate them in the card's private data structure,
   because they generally shouldn't be allocated dynamically.

   In this case, we also provide a flag to indicate if a device is
   "stopped" due to a power management event, or card ejection.  The
   device IO routines can use a flag like this to throttle IO to a
   card that is not ready to accept it.

   The bus_operations pointer is used on platforms for which we need
   to use special socket-specific versions of normal IO primitives
   (inb, outb, readb, writeb, etc) for card IO.
*/
   
typedef struct local_info_t {
    dev_link_t		link;
    dev_node_t		node;
    int			stop;
    struct bus_operations *bus;
} local_info_t;

/*====================================================================*/

static void cs_error(client_handle_t handle, int func, int ret)
{
    error_info_t err = { func, ret };
    CardServices(ReportError, handle, &err);
}

/*======================================================================

    ni_gpib_attach() creates an "instance" of the driver, allocating
    local data structures for one device.  The device is registered
    with Card Services.

    The dev_link structure is initialized, but we don't actually
    configure the card at this point -- we wait until we receive a
    card insertion event.
    
======================================================================*/

static dev_link_t *ni_gpib_attach(void)
{
    local_info_t *local;
    dev_link_t *link;
    client_reg_t client_reg;
    int ret, i;
    
    DEBUG(0, "ni_gpib_attach()\n");

    /* Allocate space for private device-specific data */
    local = kmalloc(sizeof(local_info_t), GFP_KERNEL);
    if (!local) return NULL;
    memset(local, 0, sizeof(local_info_t));
    link = &local->link; link->priv = local;
    
    /* Initialize the dev_link_t structure */
    link->release.function = &ni_gpib_release;
    link->release.data = (u_long)link;

    /* Interrupt setup */
    link->irq.Attributes = IRQ_TYPE_EXCLUSIVE;
    link->irq.IRQInfo1 = IRQ_INFO2_VALID|IRQ_LEVEL_ID;
    if (irq_list[0] == -1)
	link->irq.IRQInfo2 = irq_mask;
    else
	for (i = 0; i < 4; i++)
	    link->irq.IRQInfo2 |= 1 << irq_list[i];
    link->irq.Handler = NULL;
    
    /*
      General socket configuration defaults can go here.  In this
      client, we assume very little, and rely on the CIS for almost
      everything.  In most clients, many details (i.e., number, sizes,
      and attributes of IO windows) are fixed by the nature of the
      device, and can be hard-wired here.
    */
    link->conf.Attributes = 0;
    link->conf.Vcc = 50;
    link->conf.IntType = INT_MEMORY_AND_IO;

    /* Register with Card Services */
    link->next = dev_list;
    dev_list = link;
    client_reg.dev_info = &dev_info;
    client_reg.Attributes = INFO_IO_CLIENT | INFO_CARD_SHARE;
    client_reg.EventMask =
	CS_EVENT_CARD_INSERTION | CS_EVENT_CARD_REMOVAL |
	CS_EVENT_RESET_PHYSICAL | CS_EVENT_CARD_RESET |
	CS_EVENT_PM_SUSPEND | CS_EVENT_PM_RESUME;
    client_reg.event_handler = &ni_gpib_event;
    client_reg.Version = 0x0210;
    client_reg.event_callback_args.client_data = link;
    ret = CardServices(RegisterClient, &link->handle, &client_reg);
    if (ret != CS_SUCCESS) {
	cs_error(link->handle, RegisterClient, ret);
	ni_gpib_detach(link);
	return NULL;
    }

    return link;
} /* ni_gpib_attach */

/*======================================================================

    This deletes a driver "instance".  The device is de-registered
    with Card Services.  If it has been released, all local data
    structures are freed.  Otherwise, the structures will be freed
    when the device is released.

======================================================================*/

static void ni_gpib_detach(dev_link_t *link)
{
    dev_link_t **linkp;

    DEBUG(0, "ni_gpib_detach(0x%p)\n", link);
    
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
	printk(KERN_DEBUG "ni_gpib_cs: detach postponed, '%s' "
	       "still locked\n", link->dev->dev_name);
#endif
	link->state |= DEV_STALE_LINK;
	return;
    }

    /* Break the link with Card Services */
    if (link->handle)
	CardServices(DeregisterClient, link->handle);
    
    /* Unlink device structure, and free it */
    *linkp = link->next;
    /* This points to the parent local_info_t struct */
    kfree(link->priv);
    
} /* ni_gpib_detach */

/*======================================================================

    ni_gpib_config() is scheduled to run after a CARD_INSERTION event
    is received, to configure the PCMCIA socket, and to make the
    device available to the system.

======================================================================*/

#define CS_CHECK(fn, args...) \
while ((last_ret=CardServices(last_fn=(fn),args))!=0) goto cs_failed

#define CFG_CHECK(fn, args...) \
if (CardServices(fn, args) != 0) goto next_entry

static void ni_gpib_config(dev_link_t *link)
{
    client_handle_t handle = link->handle;
    local_info_t *dev = link->priv;
    tuple_t tuple;
    cisparse_t parse;
    int last_fn, last_ret;
    u_char buf[64];
    config_info_t conf;
    win_req_t req;
    memreq_t map;
    cistpl_cftable_entry_t dflt = { 0 };

    DEBUG(0, "ni_gpib_config(0x%p)\n", link);

    /*
       This reads the card's CONFIG tuple to find its configuration
       registers.
    */
    tuple.DesiredTuple = CISTPL_CONFIG;
    tuple.Attributes = 0;
    tuple.TupleData = buf;
    tuple.TupleDataMax = sizeof(buf);
    tuple.TupleOffset = 0;
    CS_CHECK(GetFirstTuple, handle, &tuple);
    CS_CHECK(GetTupleData, handle, &tuple);
    CS_CHECK(ParseTuple, handle, &tuple, &parse);
    link->conf.ConfigBase = parse.config.base;
    link->conf.Present = parse.config.rmask[0];

    /* Configure card */
    link->state |= DEV_CONFIG;

    /* Look up the current Vcc */
    CS_CHECK(GetConfigurationInfo, handle, &conf);
    link->conf.Vcc = conf.Vcc;

    /*
      In this loop, we scan the CIS for configuration table entries,
      each of which describes a valid card configuration, including
      voltage, IO window, memory window, and interrupt settings.

      We make no assumptions about the card to be configured: we use
      just the information available in the CIS.  In an ideal world,
      this would work for any PCMCIA card, but it requires a complete
      and accurate CIS.  In practice, a driver usually "knows" most of
      these things without consulting the CIS, and most client drivers
      will only use the CIS to fill in implementation-defined details.
    */
    tuple.DesiredTuple = CISTPL_CFTABLE_ENTRY;
    CS_CHECK(GetFirstTuple, handle, &tuple);
    while (1) {
	cistpl_cftable_entry_t *cfg = &(parse.cftable_entry);
	CFG_CHECK(GetTupleData, handle, &tuple);
	CFG_CHECK(ParseTuple, handle, &tuple, &parse);

	if (cfg->flags & CISTPL_CFTABLE_DEFAULT) dflt = *cfg;
	if (cfg->index == 0) goto next_entry;
	link->conf.ConfigIndex = cfg->index;

	/* Does this card need audio output? */
	if (cfg->flags & CISTPL_CFTABLE_AUDIO) {
	    link->conf.Attributes |= CONF_ENABLE_SPKR;
	    link->conf.Status = CCSR_AUDIO_ENA;
	}

	/* Use power settings for Vcc and Vpp if present */
	/*  Note that the CIS values need to be rescaled */
	if (cfg->vcc.present & (1<<CISTPL_POWER_VNOM)) {
	    if (conf.Vcc != cfg->vcc.param[CISTPL_POWER_VNOM]/10000)
		goto next_entry;
	} else if (dflt.vcc.present & (1<<CISTPL_POWER_VNOM)) {
	    if (conf.Vcc != dflt.vcc.param[CISTPL_POWER_VNOM]/10000)
		goto next_entry;
	}

	if (cfg->vpp1.present & (1<<CISTPL_POWER_VNOM))
	    link->conf.Vpp1 = link->conf.Vpp2 =
		cfg->vpp1.param[CISTPL_POWER_VNOM]/10000;
	else if (dflt.vpp1.present & (1<<CISTPL_POWER_VNOM))
	    link->conf.Vpp1 = link->conf.Vpp2 =
		dflt.vpp1.param[CISTPL_POWER_VNOM]/10000;

	/* Do we need to allocate an interrupt? */
	if (cfg->irq.IRQInfo1 || dflt.irq.IRQInfo1)
	    link->conf.Attributes |= CONF_ENABLE_IRQ;

	/* IO window settings */
	link->io.NumPorts1 = link->io.NumPorts2 = 0;
	if ((cfg->io.nwin > 0) || (dflt.io.nwin > 0)) {
	    cistpl_io_t *io = (cfg->io.nwin) ? &cfg->io : &dflt.io;
	    link->io.Attributes1 = IO_DATA_PATH_WIDTH_AUTO;
	    if (!(io->flags & CISTPL_IO_8BIT))
		link->io.Attributes1 = IO_DATA_PATH_WIDTH_16;
	    if (!(io->flags & CISTPL_IO_16BIT))
		link->io.Attributes1 = IO_DATA_PATH_WIDTH_8;
	    link->io.IOAddrLines = io->flags & CISTPL_IO_LINES_MASK;
	    link->io.BasePort1 = io->win[0].base;
	    link->io.NumPorts1 = io->win[0].len;
	    if (io->nwin > 1) {
		link->io.Attributes2 = link->io.Attributes1;
		link->io.BasePort2 = io->win[1].base;
		link->io.NumPorts2 = io->win[1].len;
	    }
	    /* This reserves IO space but doesn't actually enable it */
	    CFG_CHECK(RequestIO, link->handle, &link->io);
	}

	/*
	  Now set up a common memory window, if needed.  There is room
	  in the dev_link_t structure for one memory window handle,
	  but if the base addresses need to be saved, or if multiple
	  windows are needed, the info should go in the private data
	  structure for this device.

	  Note that the memory window base is a physical address, and
	  needs to be mapped to virtual space with ioremap() before it
	  is used.
	*/
	if ((cfg->mem.nwin > 0) || (dflt.mem.nwin > 0)) {
	    cistpl_mem_t *mem =
		(cfg->mem.nwin) ? &cfg->mem : &dflt.mem;
	    req.Attributes = WIN_DATA_WIDTH_16|WIN_MEMORY_TYPE_CM;
	    req.Attributes |= WIN_ENABLE;
	    req.Base = mem->win[0].host_addr;
	    req.Size = mem->win[0].len;
	    if (req.Size < 0x1000)
		req.Size = 0x1000;
	    req.AccessSpeed = 0;
	    link->win = (window_handle_t)link->handle;
	    CFG_CHECK(RequestWindow, &link->win, &req);
	    map.Page = 0; map.CardOffset = mem->win[0].card_addr;
	    CFG_CHECK(MapMemPage, link->win, &map);
	}
	/* If we got this far, we're cool! */
	break;

    next_entry:
	if (link->io.NumPorts1)
	    CardServices(ReleaseIO, link->handle, &link->io);
	CS_CHECK(GetNextTuple, handle, &tuple);
    }

    /*
       Allocate an interrupt line.  Note that this does not assign a
       handler to the interrupt, unless the 'Handler' member of the
       irq structure is initialized.
    */
    if (link->conf.Attributes & CONF_ENABLE_IRQ)
	CS_CHECK(RequestIRQ, link->handle, &link->irq);

    /*
       This actually configures the PCMCIA socket -- setting up
       the I/O windows and the interrupt mapping, and putting the
       card and host interface into "Memory and IO" mode.
    */
    CS_CHECK(RequestConfiguration, link->handle, &link->conf);

    /*
      We can release the IO port allocations here, if some other
      driver for the card is going to loaded, and will expect the
      ports to be available.
    */
    if (free_ports) {
	if (link->io.BasePort1)
	    release_region(link->io.BasePort1, link->io.NumPorts1);
	if (link->io.BasePort2)
	    release_region(link->io.BasePort2, link->io.NumPorts2);
    }

    /*
      At this point, the dev_node_t structure(s) need to be
      initialized and arranged in a linked list at link->dev.
    */
    sprintf(dev->node.dev_name, "ni_pcmcia_gpib");
    dev->node.major = dev->node.minor = 0;
    link->dev = &dev->node;

    /* Finally, report what we've done */
    printk(KERN_INFO "%s: index 0x%02x: Vcc %d.%d",
	   dev->node.dev_name, link->conf.ConfigIndex,
	   link->conf.Vcc/10, link->conf.Vcc%10);
    if (link->conf.Vpp1)
	printk(", Vpp %d.%d", link->conf.Vpp1/10, link->conf.Vpp1%10);
    if (link->conf.Attributes & CONF_ENABLE_IRQ)
	printk(", irq %d", link->irq.AssignedIRQ);
    if (link->io.NumPorts1)
	printk(", io 0x%04x-0x%04x", link->io.BasePort1,
	       link->io.BasePort1+link->io.NumPorts1-1);
    if (link->io.NumPorts2)
	printk(" & 0x%04x-0x%04x", link->io.BasePort2,
	       link->io.BasePort2+link->io.NumPorts2-1);
    if (link->win)
	printk(", mem 0x%06lx-0x%06lx", req.Base,
	       req.Base+req.Size-1);
    printk("\n");

    link->state &= ~DEV_CONFIG_PENDING;
    return;

cs_failed:
    cs_error(link->handle, last_fn, last_ret);
    ni_gpib_release((u_long)link);

} /* ni_gpib_config */

/*======================================================================

    After a card is removed, ni_gpib_release() will unregister the
    device, and release the PCMCIA configuration.  If the device is
    still open, this will be postponed until it is closed.

======================================================================*/

static void ni_gpib_release(u_long arg)
{
    dev_link_t *link = (dev_link_t *)arg;

    DEBUG(0, "ni_gpib_release(0x%p)\n", link);

    /*
       If the device is currently in use, we won't release until it
       is actually closed, because until then, we can't be sure that
       no one will try to access the device or its data structures.
    */
    if (link->open) {
	DEBUG(1, "ni_gpib_cs: release postponed, '%s' still open\n",
	      link->dev->dev_name);
	link->state |= DEV_STALE_CONFIG;
	return;
    }

    /* Unlink the device chain */
    link->dev = NULL;

    /*
      In a normal driver, additional code may be needed to release
      other kernel data structures associated with this device.
    */

    /* Don't bother checking to see if these succeed or not */
    if (link->win)
	CardServices(ReleaseWindow, link->win);
    CardServices(ReleaseConfiguration, link->handle);
    if (link->io.NumPorts1)
	CardServices(ReleaseIO, link->handle, &link->io);
    if (link->irq.AssignedIRQ)
	CardServices(ReleaseIRQ, link->handle, &link->irq);
    link->state &= ~DEV_CONFIG;

    if (link->state & DEV_STALE_LINK)
	ni_gpib_detach(link);

} /* ni_gpib_release */

/*======================================================================

    The card status event handler.  Mostly, this schedules other
    stuff to run after an event is received.

    When a CARD_REMOVAL event is received, we immediately set a
    private flag to block future accesses to this device.  All the
    functions that actually access the device should check this flag
    to make sure the card is still present.

======================================================================*/

static int ni_gpib_event(event_t event, int priority,
		       event_callback_args_t *args)
{
    dev_link_t *link = args->client_data;
    local_info_t *dev = link->priv;
    
    DEBUG(1, "ni_gpib_event(0x%06x)\n", event);
    
    switch (event) {
    case CS_EVENT_CARD_REMOVAL:
	link->state &= ~DEV_PRESENT;
	if (link->state & DEV_CONFIG) {
	    ((local_info_t *)link->priv)->stop = 1;
	    mod_timer(&link->release, jiffies + HZ/20);
	}
	break;
    case CS_EVENT_CARD_INSERTION:
	link->state |= DEV_PRESENT | DEV_CONFIG_PENDING;
	dev->bus = args->bus;
	ni_gpib_config(link);
	break;
    case CS_EVENT_PM_SUSPEND:
	link->state |= DEV_SUSPEND;
	/* Fall through... */
    case CS_EVENT_RESET_PHYSICAL:
	/* Mark the device as stopped, to block IO until later */
	dev->stop = 1;
	if (link->state & DEV_CONFIG)
	    CardServices(ReleaseConfiguration, link->handle);
	break;
    case CS_EVENT_PM_RESUME:
	link->state &= ~DEV_SUSPEND;
	/* Fall through... */
    case CS_EVENT_CARD_RESET:
	if (link->state & DEV_CONFIG)
	    CardServices(RequestConfiguration, link->handle, &link->conf);
	dev->stop = 0;
	/*
	  In a normal driver, additional code may go here to restore
	  the device state and restart IO. 
	*/
	break;
    }
    return 0;
} /* ni_gpib_event */

/*====================================================================*/

int __init init_ni_gpib_cs(void)
{
    servinfo_t serv;
    DEBUG(0, "%s\n", version);
    CardServices(GetCardServicesInfo, &serv);
    if (serv.Revision != CS_RELEASE_CODE) {
	printk( "ni_gpib_cs: Card Services release "
	       "does not match!  Revision = %i, CS_RELEASE_CODE = %i\n", serv.Revision, CS_RELEASE_CODE );
	return -EINVAL;
    }
    register_pccard_driver(&dev_info, &ni_gpib_attach, &ni_gpib_detach);
    return 0;
}

void __exit exit_ni_gpib_cs(void)
{
    DEBUG(0, "ni_gpib_cs: unloading\n");
    unregister_pccard_driver(&dev_info);
    while (dev_list != NULL) {
	del_timer(&dev_list->release);
	if (dev_list->state & DEV_CONFIG)
	    ni_gpib_release((u_long)dev_list);
	ni_gpib_detach(dev_list);
    }
}

int ni_pcmcia_attach(gpib_board_t *board);
void ni_pcmcia_detach(gpib_board_t *board);

gpib_interface_t ni_pcmcia_interface =
{
	name: "ni_pcmcia",
	attach: ni_pcmcia_attach,
	detach: ni_pcmcia_detach,
	read: tnt4882_read,
	write: tnt4882_write,
	command: tnt4882_command,
	take_control: tnt4882_take_control,
	go_to_standby: tnt4882_go_to_standby,
	request_system_control: tnt4882_request_system_control,
	interface_clear: tnt4882_interface_clear,
	remote_enable: tnt4882_remote_enable,
	enable_eos: tnt4882_enable_eos,
	disable_eos: tnt4882_disable_eos,
	parallel_poll: tnt4882_parallel_poll,
	parallel_poll_response: tnt4882_parallel_poll_response,
	line_status: tnt4882_line_status,
	update_status: tnt4882_update_status,
	primary_address: tnt4882_primary_address,
	secondary_address: tnt4882_secondary_address,
	serial_poll_response: tnt4882_serial_poll_response,
	serial_poll_status: tnt4882_serial_poll_status,
	provider_module: &__this_module,
};

int ni_pcmcia_attach(gpib_board_t *board)
{
	tnt4882_private_t *tnt_priv;
	nec7210_private_t *nec_priv;
	int isr_flags = SA_SHIRQ;

	if( dev_list == NULL )
	{
		printk( "gpib: no NI PCMCIA board found\n" );
		return -1;
	}

	board->status = 0;

	if(tnt4882_allocate_private(board))
		return -ENOMEM;
	tnt_priv = board->private_data;
	tnt_priv->io_write = outb_wrapper;
	tnt_priv->io_read = inb_wrapper;
	nec_priv = &tnt_priv->nec7210_priv;
	nec_priv->read_byte = nec7210_ioport_read_byte;
	nec_priv->write_byte = nec7210_ioport_write_byte;
	nec_priv->offset = atgpib_reg_offset;

	GPIB_DPRINTK( "ioport1 window attributes: 0x%x\n", dev_list->io.Attributes1 );
	nec_priv->iobase = dev_list->io.BasePort1;

	// get irq
	if( request_irq( dev_list->irq.AssignedIRQ, tnt4882_interrupt, isr_flags, "tnt4882", board))
	{
		printk("gpib: can't request IRQ %d\n", board->ibirq);
		return -1;
	}
	tnt_priv->irq = dev_list->irq.AssignedIRQ;

	tnt4882_init( tnt_priv, board );

	return 0;
}

void ni_pcmcia_detach(gpib_board_t *board)
{
	tnt4882_private_t *tnt_priv = board->private_data;
	nec7210_private_t *nec_priv;

	if(tnt_priv)
	{
		nec_priv = &tnt_priv->nec7210_priv;
		if(tnt_priv->irq)
		{
			free_irq(tnt_priv->irq, board);
		}
		if(nec_priv->iobase)
		{
			nec7210_board_reset( nec_priv, board );
		}
	}
	tnt4882_free_private(board);
}

#endif	// GPIB_CONFIG_PCMCIA
