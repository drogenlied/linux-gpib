//#include <ibsys.h>
#include "board.h"
#include <linux/wait.h>
#include <asm/bitops.h>

extern uint8       CurHSMode;

volatile int write_in_progress = 0;
volatile int command_out_ready = 0;

DECLARE_WAIT_QUEUE_HEAD(nec7210_read_wait);
DECLARE_WAIT_QUEUE_HEAD(nec7210_write_wait);
DECLARE_WAIT_QUEUE_HEAD(nec7210_status_wait);

/*
 * GPIB interrupt service routine -- fast and simple
 */
void nec7210_interrupt(int irq, void *arg, struct pt_regs *registerp )
{
	int status1, status2, address_status;
	gpib_char_t data;
	int ret;

	// read interrupt status (also clears status)

	status1 = GPIBin(ISR1);
	status2 = GPIBin(ISR2);

#ifdef NIPCIIa
	/* clear interrupt circuit */
	outb(0xff , CLEAR_INTR_REG(ibirq) );
#endif

	// record service request in ibsta
	if(status2 & HR_SRQI)
		set_bit(SRQI_NUM, &ibsta);

	// record address status change in ibsta
	if(status2 & HR_ADSC)
	{
		address_status = GPIBin(ADSR);
		// check if we are controller in charge
		if(address_status & HR_CIC)
			set_bit(CIC_NUM, &ibsta);
		else
			clear_bit(CIC_NUM, &ibsta);
		// check for talker/listener addressed
		if(address_status & HR_TA)
			set_bit(TACS_NUM, &ibsta);
		else
			clear_bit(TACS_NUM, &ibsta);
		if(address_status & HR_LA)
			set_bit(LACS_NUM, &ibsta);
		else
			clear_bit(LACS_NUM, &ibsta);
		// check for ~attention
		if(address_status & HR_NATN)
			clear_bit(ATN_NUM, &ibsta);
		else
			set_bit(ATN_NUM, &ibsta);
		wake_up_interruptible(&nec7210_status_wait); /* wake up sleeping process */
	}

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

	// outgoing command can be sent
	if(status2 & HR_CO)
	{
		set_bit(0, &command_out_ready);
		wake_up_interruptible(&nec7210_write_wait); /* wake up sleeping process */
	}

	// command pass through received
	if(status1 & HR_CPT)
	{
		printk("gpib command pass thru 0x%x\n", GPIBin(CPTR));
	}

//	printk("isr1 0x%x, isr2 0x%x, ibsta 0x%x\n", status1, status2, ibsta);

}


