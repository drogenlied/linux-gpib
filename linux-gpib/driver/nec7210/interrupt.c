//#include <ibsys.h>
#include "board.h"
#include <linux/wait.h>
#include <asm/bitops.h>

extern uint8       CurHSMode;

volatile int write_in_progress = 0;

DECLARE_WAIT_QUEUE_HEAD(nec7210_read_wait);
DECLARE_WAIT_QUEUE_HEAD(nec7210_write_wait);

/*
 * GPIB interrupt service routine -- fast and simple
 */
void nec7210_interrupt(int irq, void *arg, struct pt_regs *registerp )
{
	int status1, status2;
	gpib_char_t data;
	int ret;

#ifdef NIPCIIa
	/* clear interrupt circuit */
	outb(0xff , CLEAR_INTR_REG(ibirq) );
#endif

	// read interrupt status (also clears status)

	status1 = GPIBin(ISR1);
	status2 = GPIBin(ISR2);

	// handle service request
	if(status2 & HR_SRQI)
		set_bit(SRQI_NUM, &ibsta);

	// get incoming data
	if(status1 & HR_DI)
	{
		data.value = GPIBin(DIR);
		if(status1 & HR_END)
			data.end = 1;
		else
			data.end = 0;
		spin_lock(&read_buffer->lock);
		ret = gpib_buffer_put(read_buffer, data);
		spin_unlock(&read_buffer->lock);
		if(ret)
			printk("read buffer full\n");	//XXX
		wake_up_interruptible(&nec7210_read_wait); /* wake up sleeping process */
	}
	if(status1 & HR_END && (status1 & HR_DI) == 0)
		printk("bug in isr\n");

	// outgoing data can be sent
	if(status1 & HR_DO)
	{
		clear_bit(0, &write_in_progress);
		wake_up_interruptible(&nec7210_write_wait); /* wake up sleeping process */
	}

	printk("isr1 0x%x, isr2 0x%x, ibsta 0x%x\n", status1, status2, ibsta);

}


