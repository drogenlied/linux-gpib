/***************************************************************************
                          amcc5920.h  -  description
                             -------------------
  Header for amcc5920 pci chip

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

#ifndef _AMCC5920_GPIB_H
#define _AMCC5920_GPIB_H

// plx pci chip registers and bits
enum amcc_registers
{
	AMCC_INTCS_REG = 0x38,
	AMCC_PASS_THRU_REG	= 0x60,
};

enum amcc_incsr_bits
{
	AMCC_ADDON_INTR_ENABLE_BIT = 0x2000,
	AMCC_ADDON_INTR_ACTIVE_BIT = 0x400000,
	AMCC_INTR_ACTIVE_BIT = 0x800000,
};

#endif	// _AMCC5920_GPIB_H
