/***************************************************************************
                              ines/interrupt.c
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

#include "ines.h"
#include <asm/bitops.h>
#include <asm/dma.h>

/*
 * GPIB interrupt service routines
 */

void ines_interrupt(int irq, void *arg, struct pt_regs *registerp)
{
	gpib_board_t *board = arg;
	ines_private_t *priv = board->private_data;
	nec7210_private_t *nec_priv = &priv->nec7210_priv;
	unsigned int isr3_bits, isr4_bits;
	unsigned long flags;
	int wake = 0;

	if( priv->pci_chip_type == PCI_CHIP_QUANCOM )
	{
		if((inb(nec_priv->iobase + QUANCOM_IRQ_CONTROL_STATUS_REG) & QUANCOM_IRQ_ASSERTED_BIT) == 0)
			return;
		outb( QUANCOM_IRQ_ENABLE_BIT, nec_priv->iobase + QUANCOM_IRQ_CONTROL_STATUS_REG );
	}

	spin_lock_irqsave( &board->spinlock, flags );

	nec7210_interrupt( board, nec_priv );
	isr3_bits = ines_inb( priv, ISR3 );
	isr4_bits = ines_inb( priv, ISR4 );
	if( isr3_bits & IFC_ACTIVE_BIT )
	{
		push_gpib_event( board, EventIFC );
		wake++;
	}
	if( isr3_bits & FIFO_ERROR_BIT )
		printk( "ines gpib: fifo error\n" );
	if( isr3_bits & XFER_COUNT_BIT )
		wake++;

	if( isr4_bits & ( IN_FIFO_WATERMARK_BIT | IN_FIFO_FULL_BIT | OUT_FIFO_WATERMARK_BIT |
		OUT_FIFO_EMPTY_BIT ) )
		wake++;

	if( isr4_bits & IN_FIFO_FULL_BIT )
	{
		printk("ines: uh-oh, input fifo became full!\n");
		wake++;
	}
	if(wake) wake_up_interruptible(&board->wait);

	spin_unlock_irqrestore( &board->spinlock, flags );
}

