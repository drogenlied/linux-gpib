#include <ibsys.h>
extern uint8       CurHSMode;

/*
 * There is a Problem with Linux Semaphores: 
 *      - enabling Interrupts
 *      - call to down()
 *
 * could produce spurious segmentation faults, because up() is called
 * within this interrupt just before down() has been called
 * then wait will be called with empty queue.
 *
 * I worked over this code again to protect from such conditions
 * hope it works. :-)
 *
 *
 *
 */
#ifdef HP82335
extern int ccrbits;
#endif

extern uint8 ibirq;

static int IRQ_mask;

long serial=0L;

#if USEINTS
/*
 * GPIB interrupt service routine -- fast and simple
 */
void ibintr(int irq, struct pt_regs *registerp )
{
int s;

/*printk("***IRQ %ld! st=0x%x \n",serial++,GPIBin(hs_status));*/
#if DEBUG
	if (dbgMask & DBG_INTR)
	        printk("GPIB INTERRUPT! semaphore id = %d\n", espsemid.count);
#endif

#ifdef NIAT
	GPIBout(imr3, 0);		                /* disable interrupts */
#endif
#ifdef NIPCII
	GPIBout(imr2, 0);
	GPIBout(imr1, 0);
#endif
#ifdef NIPCIIa
/*
 * Seem to need both imr1 and imr2 to be  reset to avoid getting in loop.
 */
	GPIBout(imr2, 0);
	GPIBout(imr1, 0);
#if DEBUG
        if(dbgMask & DBG_INTR) printk("GPIB-IRQ nipcii\n");
#endif
        /* reset shared interrupt circuit */
	outb(0xff , ( 0x2f0 | ibirq ) );

#endif
#ifdef HP82335
	GPIBout(imr1, 0);
	GPIBout(imr0, 0);

	GPIBout(ccr, ccrbits & ~HR_INTEN);  /* disable interrupt */
        GPIBout(ccir[0x37f7], 0x0);        /* clear card interrupt IP bit */
        GPIBout(ccr, ccrbits);              /* re-enable interrupts if set */
#if DEBUG
        if(dbgMask & DBG_INTR) printk("GPIB-IRQ hp82335\n");
#endif
#endif
#if defined(CBI_PCI) || defined(MODBUS_PCI)
        pci_ResetIRQ ();
#endif

	    up(&espsemid); /* wake up sleeping process */
}


/*
 * Wait for GPIB or timer interrupt.  Semaphore will be posted by
 * either ibintr or ibtmintr.
 */



IBLCL void osWaitForInt( int imr3mask )
{
	DECLARE_WAITQUEUE(wait, current);

	DBGin("osWaitForInt");
#ifdef LINUX2_2
        if (atomic_read(&espsemid.count) <=0) {
#else
	if (espsemid.count <= 0) {
#endif
	/*
	 *	If semaphore not already available, enable
	 *	requested interrupts and wait until it is...
	 */

#ifdef LINUX2_2
         sema_init( &espsemid, 1);
#else
	  espsemid.count = 1;
#endif
          down(&espsemid); /* spurious interrups calling up() while irq's are enabled ? */

          /* now it's time to enable board interrupts */

#if defined(CBI_PCI) || defined(MODBUS_PCI)
		pci_EnableIRQ();
#endif
#if defined(CBI_4882)
		GPIBout( hs_mode, HS_CLR_SRQ_INT );
		GPIBout( hs_mode, CurHSMode );
		/*
		*/
#endif

#ifdef NIAT
	        DBGprint(DBG_DATA, ("imr3mask=0x%x  ", imr3mask));
                GPIBout(imr3, imr3mask);
#endif
#ifdef NIPCII
                DBGprint(DBG_DATA, ("imr3mask=0x%x  ", imr3mask));
		GPIBout(imr2, imr3mask); /* sorry */
#endif
#ifdef HP82335
#if 0
	        GPIBout(ccr, ccrbits & ~HR_INTEN);/* disable interrupt */
                GPIBout(ccir[0x37f7], 0x0);      /* clear card interrupt IP bit */
                GPIBout(ccr, ccrbits);            /* re-enable interrupts if set */
                DBGprint(DBG_DATA,("===>csr=0x%x",GPIBin(csr)));
#endif
                DBGprint(DBG_DATA, ("imr3mask=0x%x  ", imr3mask));
		GPIBout(imr1, imr3mask); /* sorry */
#endif
		/* now push process to sleep */
		down(&espsemid);
		up(&espsemid); /* wakeup process */

#if defined(CBI_PCI)
		printk("wait: hs_status=0x%x \n",GPIBin(hs_status));
		/*GPIBout(hs_mode, HS_CLR_HF_INT | HS_CLR_SRQ_INT | HS_CLR_EOI_INT | CurHSMode );*/
#endif

	}
		else{
#ifdef LINUX2_2
		sema_init( &espsemid, 0);
#else
		espsemid.count = 0;
		espsemid.wait  = NULL;
#endif
	}
	DBGout();
}
#endif


