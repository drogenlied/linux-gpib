#include <ibsys.h>
#include "board.h"
#include <linux/wait.h>

extern uint8       CurHSMode;

#if USEINTS

DECLARE_WAIT_QUEUE_HEAD(nec7210_wait);

/*
 * GPIB interrupt service routine -- fast and simple
 */
void nec7210_interrupt(int irq, void *arg, struct pt_regs *registerp )
{
	unsigned int status1, status2;

	status1 = GPIBin(ISR1);
	status2 = GPIBin(ISR2);

#ifdef NIPCIIa
#if DEBUG
	if(dbgMask & DBG_INTR) printk("GPIB-IRQ nipcii\n");
#endif
	/* reset shared interrupt circuit */
	outb(0xff , CLEAR_INTR_REG(ibirq) );

#endif

	wake_up_interruptible(&nec7210_wait); /* wake up sleeping process */
}


/*
 * Wait for GPIB or timer interrupt.
 */



IBLCL void waitForInt( int imr3mask )
{

	DBGin("osWaitForInt");
	if (atomic_read(&espsemid.count) <=0)
	{
		/*
		*	If semaphore not already available, enable
		*	requested interrupts and wait until it is...
		*/

		sema_init( &espsemid, 1);
		down(&espsemid); /* spurious interrups calling up() while irq's are enabled ? */

		/* now it's time to enable board interrupts */

#if defined(MODBUS_PCI)
		pci_EnableIRQ();
#endif

		DBGprint(DBG_DATA, ("imr3mask=0x%x  ", imr3mask));
		GPIBout(IMR2, imr3mask); /* sorry */

		/* now push process to sleep */
		down(&espsemid);
		up(&espsemid); /* wakeup process */

	}else
	{
		sema_init( &espsemid, 0);
	}
	DBGout();
}

#endif	// USEINTS


