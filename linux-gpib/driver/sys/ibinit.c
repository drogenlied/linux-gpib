/***************************************************************************
                               sys/ibinit.c
                             -------------------

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

#include "ibsys.h"
#include <linux/vmalloc.h>

int ibonline( gpib_board_t *board )
{
	int retval;

	if( board->online ) return -EBUSY;

	retval = gpib_allocate_board( board );
	if( retval < 0 ) return retval;

	if( board->interface->attach( board ) < 0 )
	{
		board->interface->detach(board);
		printk("gpib: interface attach failed\n");
		return -1;
	}
	board->online = 1;

	return 0;
}

/* XXX need to make sure autopoll is not in progress,
 * and board is generaly not in use (grab board lock?) */
int iboffline( gpib_board_t *board )
{
	if( board->online == 0 )
	{
		return 0;
	}

	board->interface->detach( board );
	gpib_deallocate_board( board );
	GPIB_DPRINTK( "gpib: board offline\n" );

	board->online = 0;

	return 0;
}

