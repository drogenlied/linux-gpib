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

#include "cb7210.h"
#include <asm/bitops.h>
#include <asm/dma.h>

/*
 * GPIB interrupt service routines
 */

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
