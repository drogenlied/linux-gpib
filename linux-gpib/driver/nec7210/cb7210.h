/***************************************************************************
                              nec7210/cb7210.h
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

#ifndef _CB7210_H
#define _CB7210_H

#include "amccs5933.h"

#if DMAOP && defined(CBI_PCI)
#error pci-gpib does not support ISA DMA, run make config again
#endif

// interrupt service routine
void cb_pci_interrupt(int irq, void *arg, struct pt_regs *registerp);

// base address of amccs5933 pci chip
extern unsigned long amcc_iobase;

// pci-gpib register offset
static const int cb_pci_reg_offset = 1;

// cb7210 specific registers and bits

#define HS_MODE	(0x8 * NEC7210_REG_OFFSET)	/* HS_MODE register */
#define HS_INT_LEVEL	(0x9 * NEC7210_REG_OFFSET)	/* HS_INT_LEVEL register */

#define HS_STATUS	(0x8 * NEC7210_REG_OFFSET)	/* HS_STATUS register */

/* CBI 488.2 HS control */

/* when both bit 0 and 1 are set, it
 *   1 clears the transmit state machine to an initial condition
 *   2 clears any residual interrupts left latched on cbi488.2
 *   3 resets all control bits in HS_MODE to zero
 *   4 enables TX empty interrupts
 * when both bit 0 and 1 are zero, then the high speed mode is disabled
 */
#define HS_TX_ENABLE     (1<<0)
#define HS_RX_ENABLE     (1<<1)
#define HS_HF_INT_EN     (1<<3)
#define HS_CLR_SRQ_INT   (1<<4)
#define HS_CLR_EOI_INT   (1<<5) /* RX enabled */
#define HS_CLR_EMPTY_INT (1<<5) /* TX enabled */
#define HS_CLR_HF_INT    (1<<6)
#define HS_SYS_CONTROL   (1<<7)

/* CBI 488.2 status */

#define HS_FIFO_FULL        (1<<0)
#define HS_HALF_FULL        (1<<1)
#define HS_SRQ_INT          (1<<2)
#define HS_EOI_INT          (1<<3)
#define HS_TX_MSB_NOT_EMPTY (1<<4)
#define HS_RX_MSB_NOT_EMPTY (1<<5)
#define HS_TX_LSB_NOT_EMPTY (1<<6)
#define HS_RX_LSB_NOT_EMPTY (1<<7)

/* CBI488.2 hs_int_level register */

#define HS_RESET7210    (1<<7)
#define AUX_HISPEED     0x41
#define AUX_LOSPEED     0x40


#endif _CB7210_H
