/***************************************************************************
                              ibwait.c
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

#include "gpibP.h"
#include <linux/sched.h>

/*
 * IBWAIT
 * Check or wait for a GPIB event to occur.  The mask argument
 * is a bit vector corresponding to the status bit vector.  It
 * has a bit set for each condition which can terminate the wait
 * If the mask is 0 then
 * no condition is waited for.
 */
int ibwait( gpib_board_t *board, unsigned int mask )
{
	int retval = 0;

	if (mask == 0)
	{
		return 0;
	}
	else if (mask & ~WAITBITS)
	{
		printk("bad mask 0x%x \n",mask);
		return -1;
	}
	osStartTimer( board, board->usec_timeout );
	while( ( ibstatus( board ) & mask) == 0 )
	{
		if( interruptible_sleep_on_timeout( &board->wait, 1 ) )
		{
			printk("wait interrupted\n");
			retval = -ERESTARTSYS;
			break;
		}
	}
	osRemoveTimer( board );
	return retval;
}

