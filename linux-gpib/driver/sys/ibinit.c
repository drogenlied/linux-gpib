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

int ibonline( gpib_board_t *board, gpib_file_private_t *priv )
{
	int retval;

	if( !board->online )
	{
		retval = gpib_allocate_board( board );
		if( retval < 0 ) return retval;

		if( board->interface->attach( board ) < 0 )
		{
			board->interface->detach(board);
			printk("gpib: interface attach failed\n");
			return -1;
		}
	}

	__MOD_INC_USE_COUNT( board->interface->provider_module );
	board->online++;
	priv->online_count++;

	return 0;
}

// XXX need to make sure autopoll is not in progress
int iboffline( gpib_board_t *board, gpib_file_private_t *priv )
{
	if( board->online == 0 )
	{
		return 0;
	}

	if( board->online == 1 )
	{
		board->interface->detach( board );
		gpib_deallocate_board( board );
		GPIB_DPRINTK( "gpib: board offline\n" );
	}
	__MOD_DEC_USE_COUNT( board->interface->provider_module );
	board->online--;
	priv->online_count--;

	return 0;
}

