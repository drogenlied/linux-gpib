/***************************************************************************
                              nec7210/pc2.h
                             -------------------

    begin                : Jan 2002
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

#ifndef _PC2_H
#define _PC2_H

// pc2 uses 8 consecutive io addresses
static const int pc2_iosize = 8;

// offset between io addresses of successive nec7210 registers for pc2a
static const int pc2a_reg_offset = 0x400;

//interrupt service routine
void pc2a_interrupt(int irq, void *arg, struct pt_regs *registerp);

// pc2 specific registers and bits

// interrupt clear register address
#define PC2A_CLEAR_INTR_IOBASE 0x2f0
#define CLEAR_INTR_REG(irq)	(PC2A_CLEAR_INTR_IOBASE + (irq))

#endif	// _PC2_H
