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

#include "tnt4882.h"
#include <asm/bitops.h>
#include <asm/dma.h>

/*
 * GPIB interrupt service routines
 */

void tnt4882_interrupt(int irq, void *arg, struct pt_regs *registerp)
{
	gpib_board_t *board = arg;
	tnt4882_private_t *priv = board->private_data;
	const nec7210_private_t *nec_priv = &priv->nec7210_priv;
	int isr0_bits;
	unsigned long flags;

	spin_lock_irqsave( &board->spinlock, flags );

	nec7210_interrupt(board, &priv->nec7210_priv);

	isr0_bits = priv->io_read( nec_priv->iobase + ISR0 );
	if( isr0_bits & TNT_IFCI_BIT )
	{
		push_gpib_event( &board->event_queue, EventIFC );
		wake_up_interruptible( &board->wait );
	}

	spin_unlock_irqrestore( &board->spinlock, flags );
}

