#include "board.h"
#include <linux/wait.h>
#include <asm/bitops.h>
#include <asm/dma.h>

volatile int write_in_progress = 0;
volatile int command_out_ready = 0;
volatile int dma_transfer_complete = 0;

DECLARE_WAIT_QUEUE_HEAD(nec7210_read_wait);
DECLARE_WAIT_QUEUE_HEAD(nec7210_write_wait);
DECLARE_WAIT_QUEUE_HEAD(nec7210_command_wait);
DECLARE_WAIT_QUEUE_HEAD(nec7210_status_wait);

/*
 * GPIB interrupt service routine -- fast and simple
 */
void nec7210_interrupt(int irq, void *arg, struct pt_regs *registerp )
{
	int status1, status2, address_status;
	gpib_char_t data;
	int ret;
	unsigned long flags;

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

	// get incoming data in PIO mode
	if((status1 & HR_DI) & (imr1_bits & HR_DIIE))
	{
		data.value = GPIBin(DIR);
		if(status1 & HR_END)
			data.end = 1;
		else
			data.end = 0;
		ret = gpib_buffer_put(read_buffer, data);
		if(ret)
			printk("read buffer full\n");	//XXX
		wake_up_interruptible(&nec7210_read_wait); /* wake up sleeping process */
	}

	// check for dma read transfer complete
	if((status1 & HR_END) && (imr2_bits & HR_DMAI))
	{
		set_bit(0, &dma_transfer_complete);
		wake_up_interruptible(&nec7210_read_wait); /* wake up sleeping process */
	}

	if((status1 & HR_DO) && test_bit(0, &write_in_progress))
	{
		// write data, pio mode
		if((imr2_bits & HR_DMAO) == 0)
		{
			if(gpib_buffer_get(write_buffer, &data))
			{	// no data left so we are done with write
				clear_bit(0, &write_in_progress);
				wake_up_interruptible(&nec7210_write_wait); /* wake up sleeping process */
			}else	// else write data to output
			{
				GPIBout(CDOR, data.value);
			}
		}else	// write data, isa dma mode
		{
			// check if dma transfer is complete
			flags = claim_dma_lock();
			clear_dma_ff(ibdma);
			if(get_dma_residue(ibdma) == 0)
			{
				clear_bit(0, &write_in_progress);
				wake_up_interruptible(&nec7210_write_wait); /* wake up sleeping process */
			}
			release_dma_lock(flags);
		}
	}

	// outgoing command can be sent
	if(status2 & HR_CO)
	{
		set_bit(0, &command_out_ready);
		wake_up_interruptible(&nec7210_command_wait); /* wake up sleeping process */
	}else
		clear_bit(0, &command_out_ready);

	// command pass through received
	if(status1 & HR_CPT)
	{
		printk("gpib command pass thru 0x%x\n", GPIBin(CPTR));
	}

	// output byte has been lost
	if(status1 & HR_ERR)
	{
		printk("gpib output error\n");
	}

//	printk("isr1 0x%x, isr2 0x%x, ibsta 0x%x\n", status1, status2, ibsta);

}


