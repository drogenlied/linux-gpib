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

void pc2_interrupt(int irq, void *arg, struct pt_regs *registerp)
{
	gpib_device_t *device = arg;
	pc2_private_t *priv = device->private_data;

	nec7210_interrupt(device, &priv->nec7210_priv);
}

void pc2a_interrupt(int irq, void *arg, struct pt_regs *registerp)
{
	gpib_device_t *device = arg;
	pc2_private_t *priv = device->private_data;

	nec7210_interrupt(device, &priv->nec7210_priv);

	/* clear interrupt circuit */
	outb(0xff , CLEAR_INTR_REG(priv->irq) );
}

void cb_pci_interrupt(int irq, void *arg, struct pt_regs *registerp )
{
	int bits;
	gpib_device_t *device = arg;
	cb7210_private_t *priv = device->private_data;

	// read incoming mailbox to clear mailbox full flag
	inl(priv->amcc_iobase + INCOMING_MAILBOX_REG(3));
	// clear amccs5933 interrupt
	bits = INBOX_FULL_INTR_BIT | INBOX_BYTE_BITS(3) | INBOX_SELECT_BITS(3) |
		INBOX_INTR_CS_BIT;
	outl(bits, priv->amcc_iobase + INTCSR_REG );

	cb7210_interrupt(irq, arg, registerp);
}

void cb7210_interrupt(int irq, void *arg, struct pt_regs *registerp )
{
	int hs_status;
	gpib_device_t *device = arg;
	cb7210_private_t *priv = device->private_data;
	nec7210_private_t *nec_priv = &priv->nec7210_priv;

	if((hs_status = inb(nec_priv->iobase + HS_STATUS)))
	{
		outb(HS_CLR_SRQ_INT | HS_CLR_EOI_INT |
			HS_CLR_EMPTY_INT | HS_CLR_HF_INT, nec_priv->iobase + HS_MODE);
// printk("gpib: cbi488 interrupt 0x%x\n", hs_status);
	}

	nec7210_interrupt(device, nec_priv);
}

void nec7210_interrupt(gpib_device_t *device, nec7210_private_t *priv)
{
	int status1, status2, address_status;
	unsigned long flags;

	// read interrupt status (also clears status)
	status1 = priv->read_byte(priv, ISR1);
	status2 = priv->read_byte(priv, ISR2);

	// record service request in status
	if(status2 & HR_SRQI)
	{
		set_bit(SRQI_NUM, &device->status);
		wake_up_interruptible(&device->wait);
	}

	// change in lockout status
	if(status2 & HR_LOKC)
	{
		if(status2 & HR_LOK)
			set_bit(LOK_NUM, &device->status);
		else
			clear_bit(LOK_NUM, &device->status);
	}

	// change in remote status
	if(status2 & HR_REMC)
	{
		if(status2 & HR_REM)
			set_bit(REM_NUM, &device->status);
		else
			clear_bit(REM_NUM, &device->status);
	}

	// record address status change in status
	if(status2 & HR_ADSC)
	{
		address_status = priv->read_byte(priv, ADSR);
		// check if we are controller in charge
		if(address_status & HR_CIC)
			set_bit(CIC_NUM, &device->status);
		else
			clear_bit(CIC_NUM, &device->status);
		// check for talker/listener addressed
		if(address_status & HR_TA)
			set_bit(TACS_NUM, &device->status);
		else
			clear_bit(TACS_NUM, &device->status);
		if(address_status & HR_LA)
			set_bit(LACS_NUM, &device->status);
		else
			clear_bit(LACS_NUM, &device->status);
		if(address_status & HR_NATN)
			clear_bit(ATN_NUM, &device->status);
		else
			set_bit(ATN_NUM, &device->status);
		wake_up_interruptible(&device->wait); /* wake up sleeping process */
	}

	// record reception of END
	if(status1 & HR_END)
	{
		set_bit(RECEIVED_END_BN, &priv->state);
	}

	// get incoming data in PIO mode
	if((status1 & HR_DI))
	{
		set_bit(READ_READY_BN, &priv->state);
		wake_up_interruptible(&device->wait); /* wake up sleeping process */
	}

	// check for dma read transfer complete
	if(test_bit(DMA_IN_PROGRESS_BN, &priv->state))
	{
		flags = claim_dma_lock();
		disable_dma(priv->dma_channel);
		clear_dma_ff(priv->dma_channel);
		if((status1 & HR_END) || get_dma_residue(priv->dma_channel) == 0)
		{
			clear_bit(DMA_IN_PROGRESS_BN, &priv->state);
			wake_up_interruptible(&device->wait); /* wake up sleeping process */
		}else
			enable_dma(priv->dma_channel);
		release_dma_lock(flags);
	}

	if((status1 & HR_DO))
	{
		if(test_bit(DMA_IN_PROGRESS_BN, &priv->state))	// write data, isa dma mode
		{
			// check if dma transfer is complete
			flags = claim_dma_lock();
			disable_dma(priv->dma_channel);
			clear_dma_ff(priv->dma_channel);
			if(get_dma_residue(priv->dma_channel) == 0)
			{
				clear_bit(DMA_IN_PROGRESS_BN, &priv->state);
				set_bit(WRITE_READY_BN, &priv->state);
				wake_up_interruptible(&device->wait); 
			}else
			{
				clear_bit(WRITE_READY_BN, &priv->state);
				enable_dma(priv->dma_channel);
			}
			release_dma_lock(flags);
		}else
		{
			set_bit(WRITE_READY_BN, &priv->state);
			wake_up_interruptible(&device->wait); 
		}
	}

	// outgoing command can be sent
	if(status2 & HR_CO)
	{
		set_bit(COMMAND_READY_BN, &priv->state);
		wake_up_interruptible(&device->wait); /* wake up sleeping process */
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
//printk("isr1 0x%x, imr1 0x%x, isr2 0x%x, imr2 0x%x, status 0x%x\n", status1, priv->imr1_bits, status2, priv->imr2_bits, device->status);
}


