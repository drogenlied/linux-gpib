#include <ibsys.h>
#include <board.h>

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

long serial=0L;

#if USEINTS
/*
 * GPIB interrupt service routine -- fast and simple
 */
IBLCL void ibintr(int irq, void *arg, struct pt_regs *registerp )
{

/*printk("***IRQ %ld! st=0x%x \n",serial++,GPIBin(hs_status));*/
	        printk("GPIB INTERRUPT! semaphore id = %d\n", atomic_read(&espsemid.count));

#ifdef NIAT
	GPIBout(IMR3, 0);		                /* disable interrupts */
#endif
#ifdef NIPCII
	GPIBout(IMR2, 0);
	GPIBout(IMR1, 0);
#endif
#ifdef NIPCIIa
/*
 * Seem to need both IMR1 and IMR2 to be  reset to avoid getting in loop.
 */
	GPIBout(IMR2, 0);
	GPIBout(IMR1, 0);
#if DEBUG
        if(dbgMask & DBG_INTR) printk("GPIB-IRQ nipcii\n");
#endif
        /* reset shared interrupt circuit */
	outb(0xff , ( 0x2f0 | ibirq ) );

#endif
#ifdef HP82335
	GPIBout(IMR1, 0);
	GPIBout(IMR0, 0);

	GPIBout(CCR, ccrbits & ~HR_INTEN);  /* disable interrupt */
        GPIBout(CCIR, 0x0);        /* clear card interrupt IP bit */
        GPIBout(CCR, ccrbits);              /* re-enable interrupts if set */
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
//	DECLARE_WAITQUEUE(wait, current);

	DBGin("osWaitForInt");
        if (atomic_read(&espsemid.count) <=0) {
	/*
	 *	If semaphore not already available, enable
	 *	requested interrupts and wait until it is...
	 */

         sema_init( &espsemid, 1);
          down(&espsemid); /* spurious interrups calling up() while irq's are enabled ? */

          /* now it's time to enable board interrupts */

#if defined(CBI_PCI) || defined(MODBUS_PCI)
		pci_EnableIRQ();
#endif
#if defined(CBI_4882)
		GPIBout( HS_MODE, HS_CLR_SRQ_INT );
		GPIBout( HS_MODE, CurHSMode );
		/*
		*/
#endif

#ifdef NIAT
	        DBGprint(DBG_DATA, ("imr3mask=0x%x  ", imr3mask));
                GPIBout(IMR3, imr3mask);
#endif
#ifdef NIPCII
                DBGprint(DBG_DATA, ("imr3mask=0x%x  ", imr3mask));
		GPIBout(IMR2, imr3mask); /* sorry */
#endif
#ifdef HP82335
#if 0
	        GPIBout(CCR, ccrbits & ~HR_INTEN);/* disable interrupt */
                GPIBout(CCIR, 0x0);      /* clear card interrupt IP bit */
                GPIBout(CCR, ccrbits);            /* re-enable interrupts if set */
                DBGprint(DBG_DATA,("===>csr=0x%x",GPIBin(csr)));
#endif
                DBGprint(DBG_DATA, ("imr3mask=0x%x  ", imr3mask));
		GPIBout(IMR1, imr3mask); /* sorry */
#endif
		/* now push process to sleep */
		down(&espsemid);
		up(&espsemid); /* wakeup process */

#if defined(CBI_PCI)
		printk("wait: hs_status=0x%x \n",GPIBin(HS_STATUS));
		/*GPIBout(HS_MODE, HS_CLR_HF_INT | HS_CLR_SRQ_INT | HS_CLR_EOI_INT | CurHSMode );*/
#endif

	}
		else{
		sema_init( &espsemid, 0);
	}
	DBGout();
}
#endif


