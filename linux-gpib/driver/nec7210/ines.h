/***************************************************************************
                          nec7210/ines.h  -  description
                             -------------------
  Header for ines GPIB boards

    copyright            : (C) 2002 by Frank Mori Hess
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

#ifndef _INES_GPIB_H
#define _INES_GPIB_H

typedef struct
{
	nec7210_private_t nec7210_priv;
	struct pci_dev *pci_device;
	// base address for plx pci chip
	unsigned long plx_iobase;
	unsigned int irq;
} ines_private_t;

// interrupt service routines
void ines_pci_interrupt(int irq, void *arg, struct pt_regs *registerp);

// offset between consecutive nec7210 registers
static const int ines_reg_offset = 1;

// plx pci chip registers and bits
enum
{
	PLX_INTCSR_REG = 0x4c,
};

enum
{
	INTCSR_ENABLE_INTR = 0x63,
	INTCSR_DISABLE_INTR = 0x62,
};

#endif _INES_GPIB_H
