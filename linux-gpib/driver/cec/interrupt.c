/***************************************************************************
                              cec/interrupt.c
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

#include "cec.h"
#include <asm/bitops.h>
#include <asm/dma.h>

/*
 * GPIB interrupt service routines
 */

void cec_interrupt(int irq, void *arg, struct pt_regs *registerp)
{
	gpib_board_t *board = arg;
	cec_private_t *priv = board->private_data;
	unsigned int plx_csr_bits;

plx_csr_bits = inl(priv->plx_iobase + PLX_INTCSR_REG); 
printk("plx intcsr 0x%x\n", plx_csr_bits);

	nec7210_interrupt(board, &priv->nec7210_priv);

// crash safety : if interrupt didn't clear, try flipping polarity then disabling
plx_csr_bits = inl(priv->plx_iobase + PLX_INTCSR_REG); 
if(plx_csr_bits & LINTR1_STATUS_BIT)
{
	if((plx_csr_bits & LINTR1_POLARITY_BIT)) 
	{
		printk("cec-gpib: ack, local interrupt 1 didn't clear.  Disabling.\n");
		plx_csr_bits &= ~LINTR1_EN_BIT;
	} else
	{
		printk("cec-gpib: ack, local interrupt 1 didn't clear.  Flipping polarity.\n");
		plx_csr_bits |= LINTR1_POLARITY_BIT;
	}	
}
if(plx_csr_bits & LINTR2_STATUS_BIT) 
{
	if((plx_csr_bits & LINTR2_POLARITY_BIT)) 
	{
		printk("cec-gpib: ack, local interrupt 2 didn't clear.  Disabling.\n");
		plx_csr_bits &= ~LINTR2_EN_BIT;
	} else
	{
		printk("cec-gpib: ack, local interrupt 2 didn't clear.  Flipping polarity.\n");
		plx_csr_bits |= LINTR2_POLARITY_BIT;
	}	
}
if(plx_csr_bits & (LINTR1_STATUS_BIT | LINTR2_STATUS_BIT)) 
	outl(plx_csr_bits, priv->plx_iobase + PLX_INTCSR_REG);
}

