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

extern unsigned long amcc_iobase;

#endif _CB7210_H
