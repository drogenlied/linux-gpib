/***************************************************************************
                          tms9914/init.c  -  description
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
#include <gpib_buffer.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/string.h>

#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

// size of modbus pci memory io region
static const int iomem_size = 0x2000;

void tms9914_board_reset(tms9914_private_t *priv)
{
	/* chip reset */
	write_byte(priv, AUX_CR | AUX_CS, AUXCR);

	/* clear registers by reading */
	read_byte(priv, CPTR);
	read_byte(priv, ISR0);
	read_byte(priv, ISR1);

	/* disable all interrupts */
	priv->imr0_bits = 0;
	write_byte(priv, priv->imr0_bits, IMR0);
	priv->imr1_bits = 0;
	write_byte(priv, priv->imr1_bits, IMR1);
	write_byte(priv, 0, SPMR);
	write_byte(priv, AUX_DAI | AUX_CS, AUXCR);

//	write_byte(priv, 0, EOSR);

	/* parallel poll unconfigure */
	write_byte(priv, 0, PPR);

	/* set GPIB address; MTA=PAD|100, MLA=PAD|040 */
	write_byte(priv, PAD & ADDRESS_MASK, ADR);
//	priv->admr_bits = HR_TRM0 | HR_TRM1;

#if 0

#if (SAD)
	/* enable secondary addressing */
	write_byte(priv, HR_ARS | (SAD & ADDRESS_MASK), ADR);
	priv->admr_bits |= HR_ADM1;
	write_byte(priv, priv->admr_bits, ADMR);
#else
	/* disable secondary addressing */
	write_byte(priv, HR_ARS | HR_DT | HR_DL, ADR);
	priv->admr_bits |= HR_ADM0;
	write_byte(priv, priv->admr_bits, ADMR);
#endif

#endif

	// request for data holdoff
	set_bit(RFD_HOLDOFF_BN, &priv->state);
	write_byte(priv, AUX_RHDF, AUXCR);
}

// wrapper for inb
uint8_t tms9914_ioport_read_byte(tms9914_private_t *priv, unsigned int register_num)
{
	return inb(priv->iobase + register_num * priv->offset);
}
// wrapper for outb
void tms9914_ioport_write_byte(tms9914_private_t *priv, uint8_t data, unsigned int register_num)
{
	if(register_num == AUXCR)
		udelay(1);
	outb(data, priv->iobase + register_num * priv->offset);
}

// wrapper for readb
uint8_t tms9914_iomem_read_byte(tms9914_private_t *priv, unsigned int register_num)
{
	return readb(priv->iobase + register_num * priv->offset);
}
// wrapper for writeb
void tms9914_iomem_write_byte(tms9914_private_t *priv, uint8_t data, unsigned int register_num)
{
	if(register_num == AUXCR)
		udelay(1);
	writeb(data, priv->iobase + register_num * priv->offset);
}

int init_module(void)
{
	return 0;
}

void cleanup_module(void)
{
}

EXPORT_SYMBOL(tms9914_board_reset);

EXPORT_SYMBOL(tms9914_ioport_read_byte);
EXPORT_SYMBOL(tms9914_ioport_write_byte);
EXPORT_SYMBOL(tms9914_iomem_read_byte);
EXPORT_SYMBOL(tms9914_iomem_write_byte);

