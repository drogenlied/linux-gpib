//#include <ibsys.h>
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
	int status1, status2;

#ifdef NIPCIIa
	/* clear interrupt circuit */
	outb(0xff , CLEAR_INTR_REG(ibirq) );
#endif

	// read interrupt status (also clears status)

	status1 = GPIBin(ISR1);
	status2 = GPIBin(ISR2);

	printk("isr1 0x%x, isr2 0x%x\n", status1, status2);

	wake_up_interruptible(&nec7210_wait); /* wake up sleeping process */
}



#endif	// USEINTS


