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

	nec7210_interrupt( board, nec_priv );

	spin_lock( &board->spinlock );
	isr3_bits = inb( nec_priv->iobase + ISR3 );
	isr4_bits = inb( nec_priv->iobase + ISR4 );
	if( isr3_bits & IFC_ACTIVE_BIT )
	{
		push_gpib_event( &board->event_queue, EventIFC );
		wake_up_interruptible( &board->wait );
	}
	if( isr3_bits & FIFO_ERROR_BIT )
		printk( "ines gpib: fifo error\n" );
	if( isr3_bits & XFER_COUNT_BIT )
		wake_up_interruptible( &board->wait );

	if( isr4_bits & ( IN_FIFO_WATERMARK_BIT | OUT_FIFO_WATERMARK_BIT | OUT_FIFO_EMPTY_BIT ) )
		wake_up_interruptible( &board->wait );

	spin_unlock( &board->spinlock );
}

