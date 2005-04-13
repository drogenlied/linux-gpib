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
};

enum
{
	LINTR1_EN_BIT = 0x1,
	LINTR1_POLARITY_BIT = 0x2,
	LINTR1_STATUS_BIT = 0x4,
	LINTR2_EN_BIT = 0x8,
	LINTR2_POLARITY_BIT = 0x10,
	LINTR2_STATUS_BIT = 0x20,
	PCI_INTR_EN_BIT = 0x40,
	SOFT_INTR_BIT = 0x80,
	LINTR1_SELECT_ENABLE_BIT = 0x10,	//9052 extension
	LINTR2_SELECT_ENABLE_BIT = 0x20,	//9052 extension 
	LINTR1_EDGE_CLEAR_BIT = 0x40,	//9052 extension
	LINTR2_EDGE_CLEAR_BIT = 0x80,	//9052 extension
};

#endif	// _PLX9050_GPIB_H
