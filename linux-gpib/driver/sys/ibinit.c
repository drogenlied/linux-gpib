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

#include "gpibP.h"
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/module.h>

int ibonline( gpib_board_t *board, gpib_file_private_t *priv,
	int master )
{
	if( !board->online )
	{
		board->buffer_length = 0x1000;
		board->buffer = vmalloc( board->buffer_length );
		if(board->buffer == NULL)
			return -ENOMEM;

		if( board->interface->attach( board ) < 0 )
		{
			board->interface->detach(board);
			printk("GPIB Hardware Error! (Chip type not found or wrong Base Address?)\n");
			return -1;
		}

		if( master )
		{
			board->master = 1;
			ibsic( board, 100 );
		}else
		{
			board->master = 0;
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
		if( board->buffer )
		{
			vfree( board->buffer );
			board->buffer = NULL;
		}
	}
	__MOD_DEC_USE_COUNT( board->interface->provider_module );
	board->online--;
	priv->online_count--;

	return 0;
}

