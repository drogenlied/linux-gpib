/***************************************************************************
                          lib/ibBdChrConfig.c
                             -------------------

    copyright            : (C) 2001,2002,2003 by Frank Mori Hess
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

#include "ib_internal.h"
#include <sys/ioctl.h>
#include <string.h>

int ibBdChrConfig( ibBoard_t *board )
{
	board_type_ioctl_t boardtype;
	select_pci_ioctl_t pci_selection;
	int retval;

	if( board->fileno < 0 )
	{
		fprintf( stderr, "libgpib: bug, tried to configure unopened board\n" );
		return -1;
	}

	strncpy( boardtype.name, board->board_type, sizeof( boardtype.name ) );
	retval = ioctl( board->fileno, CFCBOARDTYPE, &boardtype );
	if( retval < 0 ) return retval;
	retval = ioctl( board->fileno, CFCBASE, &board->base );
	if( retval < 0 ) return retval;
	retval = ioctl( board->fileno, CFCIRQ, &board->irq );
	if( retval < 0 ) return retval;
	retval = ioctl( board->fileno, CFCDMA, &board->dma );
	if( retval < 0 ) return retval;
 	retval = ioctl( board->fileno, IBPAD, &board->pad );
	if( retval < 0 ) return retval;
	retval = ioctl( board->fileno, IBSAD, &board->sad );
	if( retval < 0 ) return retval;

	pci_selection.pci_bus = board->pci_bus;
	pci_selection.pci_slot = board->pci_slot;
	retval = ioctl( board->fileno, IBSELECT_PCI, &pci_selection );
	if( retval < 0 ) return retval;


	return 0;
}

