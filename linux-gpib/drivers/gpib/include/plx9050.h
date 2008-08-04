/***************************************************************************
                          plx9050.h  -  description
                             -------------------
  Header for plx9050 pci chip

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

#ifndef _PLX9050_GPIB_H
#define _PLX9050_GPIB_H

// plx pci chip registers and bits
enum
{
	PLX_INTCSR_REG = 0x4c,
	PLX_CNTRL_REG = 0x50
};

enum plx9050_intcsr_bits
{
	LINTR1_EN_BIT = 0x1,
	LINTR1_POLARITY_BIT = 0x2,
	LINTR1_STATUS_BIT = 0x4,
	LINTR2_EN_BIT = 0x8,
	LINTR2_POLARITY_BIT = 0x10,
	LINTR2_STATUS_BIT = 0x20,
	PCI_INTR_EN_BIT = 0x40,
	SOFT_INTR_BIT = 0x80,
	LINTR1_SELECT_ENABLE_BIT = 0x100,	//9052 extension
	LINTR2_SELECT_ENABLE_BIT = 0x200,	//9052 extension 
	LINTR1_EDGE_CLEAR_BIT = 0x400,	//9052 extension
	LINTR2_EDGE_CLEAR_BIT = 0x800,	//9052 extension
};

enum plx9050_cntrl_bits
{
	WAITO_NOT_USER0_SELECT_BIT = 0x1,
	USER0_OUTPUT_BIT = 0x2,
	USER0_DATA_BIT = 0x4,
	LLOCK_NOT_USER1_SELECT_BIT = 0x8,
	USER1_OUTPUT_BIT = 0x10,
	USER1_DATA_BIT = 0x20,
	CS2_NOT_USER2_SELECT_BIT = 0x40,
	USER2_OUTPUT_BIT = 0x80,
	USER2_DATA_BIT = 0x100,
	CS3_NOT_USER3_SELECT_BIT = 0x200,
	USER3_OUTPUT_BIT = 0x400,
	USER3_DATA_BIT = 0x800,
	PCIBAR_ENABLE_MASK = 0x3000,
	PCIBAR_MEMORY_AND_IO_ENABLE_BITS = 0x0,
	PCIBAR_MEMORY_NO_IO_ENABLE_BITS = 0x1000,
	PCIBAR_IO_NO_MEMORY_ENABLE_BITS = 0x2000,
	PCIBAR_MEMORY_AND_IO_TOO_ENABLE_BITS = 0x3000,
	PCI_READ_MODE_BIT = 0x4000,
	PCI_READ_WITH_WRITE_FLUSH_MODE_BIT = 0x8000,
	PCI_READ_NO_FLUSH_MODE_BIT = 0x10000,
	PCI_READ_NO_WRITE_MODE_BIT = 0x20000
};

#endif	// _PLX9050_GPIB_H
