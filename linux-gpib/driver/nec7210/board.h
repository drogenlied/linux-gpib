/***************************************************************************
                              nec7210/board.h
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


#ifndef _GPIB_PCIIA_BOARD_H
#define _GPIB_PCIIA_BOARD_H

#include <gpibP.h>
#include <asm/io.h>
#include <gpib_buffer.h>

#include "nec7210.h"
#include "pc2.h"
#include "cb7210.h"

extern unsigned long ibbase;	/* base addr of GPIB interface registers  */
extern unsigned int ibirq;	/* interrupt request line for GPIB (1-7)  */
extern unsigned int ibdma ;      /* DMA channel                            */

extern gpib_driver_t pc2_driver;
extern gpib_driver_t pc2a_driver;
extern gpib_driver_t cb_pci_driver;

#endif	//_GPIB_PCIIA_BOARD_H

