#include <ibsys.h>
#include "board.h"

extern int ccrbits;

long serial=0L;

#if USEINTS
/*
 * GPIB interrupt service routine -- fast and simple
 */
IBLCL void ibintr(int irq, void *arg, struct pt_regs *registerp )
{

/*printk("***IRQ %ld! st=0x%x \n",serial++,GPIBin(hs_status));*/
	        printk("GPIB INTERRUPT! semaphore id = %d\n", atomic_read(&espsemid.count));

	GPIBout(IMR1, 0);
	GPIBout(IMR0, 0);

	GPIBout(CCR, ccrbits & ~HR_INTEN);  /* disable interrupt */
        GPIBout(CCIR, 0x0);        /* clear card interrupt IP bit */
        GPIBout(CCR, ccrbits);              /* re-enable interrupts if set */
#if DEBUG
        if(dbgMask & DBG_INTR) printk("GPIB-IRQ hp82335\n");
#endif

	    up(&espsemid); /* wake up sleeping process */
}


/*
 * Wait for GPIB or timer interrupt.  Semaphore will be posted by
 * either ibintr or ibtmintr.
 */



IBLCL void osWaitForInt( int imr3mask )
{
	DBGin("osWaitForInt");
        if (atomic_read(&espsemid.count) <=0) {
	/*
	 *	If semaphore not already available, enable
	 *	requested interrupts and wait until it is...
	 */

         sema_init( &espsemid, 1);
          down(&espsemid); /* spurious interrups calling up() while irq's are enabled ? */

          /* now it's time to enable board interrupts */

                DBGprint(DBG_DATA, ("imr3mask=0x%x  ", imr3mask));
		GPIBout(IMR1, imr3mask); /* sorry */
		/* now push process to sleep */
		down(&espsemid);
		up(&espsemid); /* wakeup process */

	}
		else{
		sema_init( &espsemid, 0);
	}
	DBGout();
}
#endif


