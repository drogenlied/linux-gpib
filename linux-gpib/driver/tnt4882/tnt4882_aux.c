/***************************************************************************
                          nec7210/tnt4882_aux.c
                             -------------------

    begin                : 2002
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

#include "tnt4882.h"
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>

int tnt4882_line_status( const gpib_board_t *board )
{
	int status = ValidALL;
	int bcsr_bits;
	tnt4882_private_t *tnt_priv;

	tnt_priv = board->private_data;

	bcsr_bits = tnt_priv->io_read( tnt_priv->nec7210_priv.iobase + BCSR );

	if( bcsr_bits & BCSR_REN_BIT )
		status |= BusREN;
	if( bcsr_bits & BCSR_IFC_BIT )
		status |= BusIFC;
	if( bcsr_bits & BCSR_SRQ_BIT )
		status |= BusSRQ;
	if( bcsr_bits & BCSR_EOI_BIT )
		status |= BusEOI;
	if( bcsr_bits & BCSR_NRFD_BIT )
		status |= BusNRFD;
	if( bcsr_bits & BCSR_NDAC_BIT )
		status |= BusNDAC;
	if( bcsr_bits & BCSR_DAV_BIT )
		status |= BusDAV;
	if( bcsr_bits & BCSR_ATN_BIT )
		status |= BusATN;

	return status;
}
