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
	
	if(priv->amcc_iobase)
	{
		if((inl(priv->amcc_iobase + AMCC_INTCS_REG) & AMCC_ADDON_INTR_ACTIVE_BIT))
		{
			// clear amcc interrupt
			outl(AMCC_ADDON_INTR_ENABLE_BIT, priv->amcc_iobase + AMCC_INTCS_REG);
// XXX
printk("amcc status: 0x%x\n", inl(priv->amcc_iobase + AMCC_INTCS_REG));
		}
	}

	nec7210_interrupt(board, &priv->nec7210_priv);
}

