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
#include <gpib_buffer.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/string.h>

#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

// size of modbus pci memory io region
static const int iomem_size = 0x2000;

void nec7210_board_reset(nec7210_private_t *priv)
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

	/* set GPIB address; MTA=PAD|100, MLA=PAD|040 */
	write_byte(priv, PAD & ADDRESS_MASK, ADR);
	priv->admr_bits = HR_TRM0 | HR_TRM1;
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

	// holdoff on all data	XXX record current handshake state somewhere
	priv->auxa_bits = AUXRA;
	write_byte(priv, priv->auxa_bits | HR_HLDA, AUXMR);

	/* set INT pin to active high */
	write_byte(priv, AUXRB, AUXMR);
	write_byte(priv, AUXRE, AUXMR);
}

// wrapper for inb
uint8_t ioport_read_byte(nec7210_private_t *priv, unsigned int register_num)
{
	return inb(priv->iobase + register_num * priv->offset);
}
// wrapper for outb
void ioport_write_byte(nec7210_private_t *priv, uint8_t data, unsigned int register_num)
{
	outb(data, priv->iobase + register_num * priv->offset);
}

// wrapper for readb
uint8_t iomem_read_byte(nec7210_private_t *priv, unsigned int register_num)
{
	return readb(priv->iobase + register_num * priv->offset);
}
// wrapper for writeb
void iomem_write_byte(nec7210_private_t *priv, uint8_t data, unsigned int register_num)
{
	writeb(data, priv->iobase + register_num * priv->offset);
}

int init_module(void)
{
	int err = 0;

	EXPORT_NO_SYMBOLS;

	INIT_LIST_HEAD(&pc2_interface.list);
	INIT_LIST_HEAD(&pc2a_interface.list);
	INIT_LIST_HEAD(&cb_pci_interface.list);
	INIT_LIST_HEAD(&cb_isa_interface.list);
	INIT_LIST_HEAD(&ines_pci_interface.list);
	INIT_LIST_HEAD(&ni_isa_interface.list);

	gpib_register_driver(&pc2_interface);
	gpib_register_driver(&pc2a_interface);
	gpib_register_driver(&cb_pci_interface);
	gpib_register_driver(&cb_isa_interface);
	gpib_register_driver(&ines_pci_interface);
	gpib_register_driver(&ni_isa_interface);

#ifdef CONFIG_PCMCIA
	INIT_LIST_HEAD(&cb_pcmcia_interface.list);
	INIT_LIST_HEAD(&ines_pcmcia_interface.list);

	gpib_register_driver(&cb_pcmcia_interface);
	gpib_register_driver(&ines_pcmcia_interface);
	err += cb_pcmcia_init_module();
	err += ines_pcmcia_init_module();
#endif
	if(err)
		return -1;

	return 0;
}

void cleanup_module(void)
{
	gpib_unregister_driver(&pc2_interface);
	gpib_unregister_driver(&pc2a_interface);
	gpib_unregister_driver(&cb_pci_interface);
	gpib_unregister_driver(&cb_isa_interface);
	gpib_unregister_driver(&ines_pci_interface);
	gpib_unregister_driver(&ni_isa_interface);
#ifdef CONFIG_PCMCIA
	gpib_unregister_driver(&cb_pcmcia_interface);
	gpib_unregister_driver(&cb_pcmcia_interface);
	cb_pcmcia_cleanup_module();
	ines_pcmcia_cleanup_module();
#endif
}









