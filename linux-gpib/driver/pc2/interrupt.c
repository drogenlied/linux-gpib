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

#include "pc2.h"
#include <asm/bitops.h>
#include <asm/dma.h>

/*
 * GPIB interrupt service routines
 */

void pc2_interrupt(int irq, void *arg, struct pt_regs *registerp)
{
	gpib_board_t *board = arg;
	pc2_private_t *priv = board->private_data;

	nec7210_interrupt(board, &priv->nec7210_priv);
}

void pc2a_interrupt(int irq, void *arg, struct pt_regs *registerp)
{
	gpib_board_t *board = arg;
	pc2_private_t *priv = board->private_data;
	int status1, status2;

	// read interrupt status (also clears status)
	status1 = read_byte( &priv->nec7210_priv, ISR1 );
	status2 = read_byte( &priv->nec7210_priv, ISR2 );

	/* clear interrupt circuit */
	outb(0xff , CLEAR_INTR_REG(priv->irq) );

	nec7210_interrupt_have_status( board, &priv->nec7210_priv, status1, status2 );
}
