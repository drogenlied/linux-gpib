/***************************************************************************
                          nec7210/init.c  -  description
                             -------------------
 board specific initialization stuff

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
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <asm/dma.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/string.h>

#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

// size of modbus pci memory io region
static const int iomem_size = 0x2000;

void nec7210_board_reset( nec7210_private_t *priv, const gpib_board_t *board )
{
#ifdef MODBUS_PCI
	GPIBout(0x20, 0xff); /* enable controller mode */
#endif
	/* 7210 chip reset */
	write_byte(priv, AUX_CR, AUXMR);

	/* clear registers by reading */
	read_byte(priv, CPTR);
	read_byte(priv, ISR1);
	read_byte(priv, ISR2);

	/* disable all interrupts */
	priv->imr1_bits = 0;
	write_byte(priv, priv->imr1_bits, IMR1);
	priv->imr2_bits = 0;
	write_byte(priv, priv->imr2_bits, IMR2);
	write_byte(priv, 0, SPMR);

	write_byte(priv, 0, EOSR);
	/* set internal counter register 8 for 8 MHz input clock */
	write_byte(priv, ICR + 8, AUXMR);
	/* parallel poll unconfigure */
	write_byte(priv, PPR | HR_PPU, AUXMR);

	/* set GPIB address */
	nec7210_primary_address( board, priv, board->pad );
	priv->admr_bits = HR_TRM0 | HR_TRM1;

	nec7210_secondary_address( board, priv, board->sad, board->sad >= 0 );

	// holdoff on all data
	priv->auxa_bits = AUXRA;
	write_byte(priv, priv->auxa_bits | HR_HLDA, AUXMR);

	/* set INT pin to active high */
	priv->auxb_bits = AUXRB;
	write_byte(priv, priv->auxb_bits, AUXMR);
	write_byte(priv, AUXRE, AUXMR);
}

// wrapper for inb
uint8_t nec7210_ioport_read_byte(nec7210_private_t *priv, unsigned int register_num)
{
	return inb(priv->iobase + register_num * priv->offset);
}
// wrapper for outb
void nec7210_ioport_write_byte(nec7210_private_t *priv, uint8_t data, unsigned int register_num)
{
	outb(data, priv->iobase + register_num * priv->offset);
	if(register_num == AUXMR)
		udelay(1);
}

// wrapper for readb
uint8_t nec7210_iomem_read_byte(nec7210_private_t *priv, unsigned int register_num)
{
	return readb(priv->iobase + register_num * priv->offset);
}
// wrapper for writeb
void nec7210_iomem_write_byte(nec7210_private_t *priv, uint8_t data, unsigned int register_num)
{
	writeb(data, priv->iobase + register_num * priv->offset);
	if(register_num == AUXMR)
		udelay(1);
}

int init_module(void)
{
	return 0;
}

void cleanup_module(void)
{
}

EXPORT_SYMBOL(nec7210_board_reset);

EXPORT_SYMBOL(nec7210_ioport_read_byte);
EXPORT_SYMBOL(nec7210_ioport_write_byte);
EXPORT_SYMBOL(nec7210_iomem_read_byte);
EXPORT_SYMBOL(nec7210_iomem_write_byte);

