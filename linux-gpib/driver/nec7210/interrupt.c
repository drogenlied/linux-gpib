/***************************************************************************
                              nec7210/interrupt.c
                             -------------------

    begin                : Dec 2001
    copyright            : (C) 2001, 2002 by Frank Mori Hess
    email                : fmhess@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "board.h"
#include <asm/bitops.h>
#include <asm/dma.h>

/*
 * GPIB interrupt service routines
 */

void pc2a_interrupt(int irq, void *arg, struct pt_regs *registerp)
{
	nec7210_interrupt(irq, arg, registerp);

	/* clear interrupt circuit */
	outb(0xff , CLEAR_INTR_REG(ibirq) );
}

void cb_pci_interrupt(int irq, void *arg, struct pt_regs *registerp )
{
	int bits, hs_status;
	gpib_driver_t *driver = (gpib_driver_t*) arg;
	nec7210_private_t *priv = driver->private_data;

printk("ammc status 0x%x\n", inl(amcc_iobase + INTCSR_REG));

	// read incoming mailbox to clear mailbox full flag
	inl(amcc_iobase + INCOMING_MAILBOX_REG(3));
	// clear amccs5933 interrupt
	bits = INBOX_FULL_INTR_BIT | INBOX_BYTE_BITS(3) | INBOX_SELECT_BITS(3) |
		INBOX_INTR_CS_BIT;
	outl(bits, amcc_iobase + INTCSR_REG );

	if((hs_status = inb(priv->iobase + HS_STATUS)))
	{
		outb(HS_CLR_SRQ_INT | HS_CLR_EOI_INT |
			HS_CLR_EMPTY_INT | HS_CLR_HF_INT, priv->iobase + HS_MODE);
		printk("gpib: cbi488 interrupt? 0x%x\n", hs_status);
	}

	nec7210_interrupt(irq, arg, registerp);
}

void nec7210_interrupt(int irq, void *arg, struct pt_regs *registerp )
{
	int status1, status2, address_status;
	gpib_char_t data;
	int ret;
	unsigned long flags;
	gpib_driver_t *driver = (gpib_driver_t*) arg;
	nec7210_private_t *priv = driver->private_data;

	/* interrupt should also update RDF_HOLDOFF in state
	 * by checking auxa_bits and END, but I need to make
	 * auxa_bits store handshaking bits first */

	// read interrupt status (also clears status)
	status1 = priv->read_byte(priv, ISR1);
	status2 = priv->read_byte(priv, ISR2);

	// record service request in status
	if(status2 & HR_SRQI)
	{
		set_bit(SRQI_NUM, &driver->status);
		wake_up_interruptible(&driver->wait);
	}

	// change in lockout status
	if(status2 & HR_LOKC)
	{
		if(status2 & HR_LOK)
			set_bit(LOK_NUM, &driver->status);
		else
			clear_bit(LOK_NUM, &driver->status);
	}

	// change in remote status
	if(status2 & HR_REMC)
	{
		if(status2 & HR_REM)
			set_bit(REM_NUM, &driver->status);
		else
			clear_bit(REM_NUM, &driver->status);
	}

	// record address status change in status
	if(status2 & HR_ADSC)
	{
		address_status = priv->read_byte(priv, ADSR);
		// check if we are controller in charge
		if(address_status & HR_CIC)
			set_bit(CIC_NUM, &driver->status);
		else
			clear_bit(CIC_NUM, &driver->status);
		// check for talker/listener addressed
		if(address_status & HR_TA)
			set_bit(TACS_NUM, &driver->status);
		else
			clear_bit(TACS_NUM, &driver->status);
		if(address_status & HR_LA)
			set_bit(LACS_NUM, &driver->status);
		else
			clear_bit(LACS_NUM, &driver->status);
		if(address_status & HR_NATN)
			clear_bit(ATN_NUM, &driver->status);
		else
			set_bit(ATN_NUM, &driver->status);
		wake_up_interruptible(&driver->wait); /* wake up sleeping process */
	}

	// record reception of END
	if(status1 & HR_END)
		set_bit(END_NUM, &driver->status);

	// get incoming data in PIO mode
	if((status1 & HR_DI) & (priv->imr1_bits & HR_DIIE))
	{
		data.value = priv->read_byte(priv, DIR);
		if(status1 & HR_END)
			data.end = 1;
		else
			data.end = 0;
		ret = gpib_buffer_put(read_buffer, data);
		if(ret)
			printk("read buffer full\n");	//XXX
		wake_up_interruptible(&driver->wait); /* wake up sleeping process */
	}

	// check for dma read transfer complete
	if(priv->imr2_bits & HR_DMAI)
	{
		flags = claim_dma_lock();
		disable_dma(priv->dma_channel);
		clear_dma_ff(priv->dma_channel);
		if((status1 & HR_END) || get_dma_residue(priv->dma_channel) == 0)
		{
			clear_bit(DMA_IN_PROGRESS_BN, &priv->state);
			wake_up_interruptible(&driver->wait); /* wake up sleeping process */
		}else
			enable_dma(priv->dma_channel);
		release_dma_lock(flags);
	}

	if((status1 & HR_DO) && test_bit(WRITING_BN, &priv->state))
	{
		// write data, pio mode
		if((priv->imr2_bits & HR_DMAO) == 0)
		{
			if(gpib_buffer_get(write_buffer, &data))
			{	// no data left so we are done with write
				clear_bit(WRITING_BN, &priv->state);
				wake_up_interruptible(&driver->wait); /* wake up sleeping process */
			}else	// else write data to output
			{
				priv->write_byte(priv, data.value, CDOR);
			}
		}else	// write data, isa dma mode
		{
			// check if dma transfer is complete
			flags = claim_dma_lock();
			disable_dma(priv->dma_channel);
			clear_dma_ff(priv->dma_channel);
			if(get_dma_residue(priv->dma_channel) == 0)
			{
				clear_bit(WRITING_BN, &priv->state);
				wake_up_interruptible(&driver->wait); /* wake up sleeping process */
			}else
				enable_dma(priv->dma_channel);
			release_dma_lock(flags);
		}
	}

	// outgoing command can be sent
	if(status2 & HR_CO)
	{
		set_bit(COMMAND_READY_BN, &priv->state);
		wake_up_interruptible(&driver->wait); /* wake up sleeping process */
	}else
		clear_bit(COMMAND_READY_BN, &priv->state);

	// command pass through received
	if(status1 & HR_CPT)
	{
		printk("gpib command pass thru 0x%x\n", priv->read_byte(priv, CPTR));
	}

	// output byte has been lost
	if(status1 & HR_ERR)
	{
		printk("gpib output error\n");
	}

printk("isr1 0x%x, isr2 0x%x, status 0x%x\n", status1, status2, driver->status);

}


