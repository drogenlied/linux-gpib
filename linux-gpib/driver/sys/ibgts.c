/***************************************************************************
                               sys/ibgts.c
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

/*
 * IBGTS
 * Go to the controller standby state from the controller
 * active state, i.e., turn ATN off.
 */

int ibgts( gpib_board_t *board )
{
	int status = ibstatus(board);

	if( ( status & CIC ) == 0 )
	{
		printk( "gpib: not CIC during ibgts\n" );
		return -1;
	}

	board->interface->go_to_standby( board );                    /* go to standby */
	
	return 0;
}

