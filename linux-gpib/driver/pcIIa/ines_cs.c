/*======================================================================

    A gpib PCMCIA client driver

    Written by Claus Schroeter (clausi@chemie.fu-berlin.de)
    adapted from the skeleton example
 
    adapted from ../cbi4882/gpib_cs.c 
    for ines GPIB-PCMCIA card 
    13.1.99 Axel Dziemba (axel.dziemba@ines.de)
 
======================================================================*/


#include "board.h"

#ifdef INES_PCMCIA
#include <pcmcia/config.h>
#include <pcmcia/k_compat.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/malloc.h>
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
"ines_cs.c 0.10 1997/07/01 19:30:58 (Claus Schroeter)";
#endif


#define GPIB_MAJOR 31

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

void gpib_interrupt(int reg);

/*
   The dev_info variable is the "key" that is used to match up this
   device driver with appropriate cards, through the card configuration
   database.
*/

static dev_info_t dev_info = "ines_cs";

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

static void cs_error(int func, int ret)
{
    CardServices(ReportError, dev_info, (void *)func, (void *)ret);
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
    link->io.NumPorts1 =32;
    link->io.Attributes1 = IO_DATA_PATH_WIDTH_8;
    link->io.NumPorts2 = 0;
    link->io.Attributes2 =0;
    link->io.IOAddrLines = 10;

    /* Interrupt setup */
    link->irq.Attributes = IRQ_TYPE_EXCLUSIVE;
    link->irq.IRQInfo1 = IRQ_INFO2_VALID|IRQ_LEVEL_ID;
    link->irq.IRQInfo2 = irq_mask;
    link->irq.Handler = gpib_interrupt;
    
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
	cs_error(RegisterClient, ret);
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
	printk(KERN_DEBUG "ines_cs: detach postponed, '%s' "
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
	kfree_s(link->priv, sizeof(local_info_t));
    }
    kfree_s(link, sizeof(struct dev_link_t));
    
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
    int i, j;
    u_char buf[64];
    win_req_t req;
    modwin_t mod;
    memreq_t mem;
    u_char *virt;
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
	cs_error(ParseTuple, i);
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
	  printk(KERN_DEBUG "ines_cs: manufacturer: 0x%x card: 0x%x\n",
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
		printk(KERN_DEBUG "ines_cs: irqmask=0x%x\n",
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
		     printk( KERN_DEBUG "ines_cs: base=0x%x len=%d registered\n",
//	       parse.cftable_entry.io.win[0].base, // not updated! (ax)
  	               link->io.BasePort1,
		       parse.cftable_entry.io.win[0].len
		       );  
//                     ibbase = parse.cftable_entry.io.win[0].base;//not updated! (ax)
                     ibbase = link->io.BasePort1;
		     break;	
	         }
	      }
	      if ( next_tuple(handle,&tuple,&parse) != CS_SUCCESS ) break;
		 
	    }

	  if (i != CS_SUCCESS) {
	      cs_error(RequestIO, i);
	  }
	 } else {
	    printk("ines_cs: can't get card information\n");
	 }

	/*
	   Now allocate an interrupt line.  Note that this does not
	   actually assign a handler to the interrupt.
	*/
	i = CardServices(RequestIRQ, link->handle, &link->irq);
	if (i != CS_SUCCESS) {
	    cs_error(RequestIRQ, i);
	    break;
	}
        printk(KERN_DEBUG "ines_cs: IRQ_Line=%d\n",link->irq.AssignedIRQ);
        ibirq = link->irq.AssignedIRQ;	 

	/*
	   This actually configures the PCMCIA socket -- setting up
	   the I/O windows and the interrupt mapping.
	*/
	i = CardServices(RequestConfiguration, link->handle, &link->conf);
	if (i != CS_SUCCESS) {
	    cs_error(RequestConfiguration, i);
	    break;
	}
        /*  for the ines card we have to setup the configuration registers in
            attribute memory here
        */
        req.Attributes=WIN_MEMORY_TYPE_AM | WIN_DATA_WIDTH_8 | WIN_ENABLE;
        req.Base=0;
        req.Size=0x800;
        req.AccessSpeed=2; 
        i= CardServices(RequestWindow,&handle,&req);
	if (i != CS_SUCCESS) {
	    cs_error(RequestWindow, i);
	    break;
        }
        mem.CardOffset=0;
        mem.Page=0;
        i= CardServices(MapMemPage,handle,&mem);
	if (i != CS_SUCCESS) {
	    cs_error(MapMemPage, i);
	    break;
        }
        virt=ioremap(req.Base,0x800);
        writeb((link->io.BasePort1>>2) & 0xff, virt+0xf0); // IOWindow base
        writeb(0x70,virt+0x100);                  // LevlIrq, 32 byte IOWindow 
        writeb(0x20,virt+0x102);                  // IOis8
        iounmap(virt); 
        CardServices(ReleaseWindow,handle);
    } while (0);

    /* At this point, the dev_node_t structure(s) should be
       initialized and arranged in a linked list at link->dev. */
    sprintf(dev->node.dev_name, "gpib0");
    dev->node.major = GPIB_MAJOR;
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
    local_info_t *local = link->priv;

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
	    printk(KERN_DEBUG "ines_cs: release postponed, '%s' "
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
	    printk(KERN_DEBUG "ines_cs: registration complete\n");
	break;
#endif
    case CS_EVENT_CARD_REMOVAL:
	link->state &= ~DEV_PRESENT;
	if (link->state & DEV_CONFIG) {
	    /* ((local_info_t *)link->priv)->block = 1;*/
	    link->release.expires = RUN_AT(HZ/20);
	    add_timer(&link->release);
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

void gpib_interrupt(int reg)
{
    printk("ines_cs: interrupt\n");
} /* gpib_interrupt */

/*====================================================================*/

int pcmcia_init_module(void)
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

void pcmcia_cleanup_module(void)
{
#ifdef PCMCIA_DEBUG
    if (pc_debug)
	printk(KERN_DEBUG "ines_cs: unloading\n");
#endif
    unregister_pcmcia_driver(&dev_info);
    while (dev_list != NULL) {
	if (dev_list->state & DEV_CONFIG)
	    gpib_release((u_long)dev_list);
	gpib_detach(dev_list);
    }
}
#endif /*PCMCIA*/
