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
#include "autopoll.h"
#include <linux/sched.h>


static int wait_satisfied( gpib_board_t *board, gpib_device_t *device,
	unsigned int mask )
{
	if( mask & RQS )
	{
		if( num_status_bytes( device ) ) return 1;
	}
	if( mask & ibstatus( board ) ) return 1;

	return 0;
}

/*
 * IBWAIT
 * Check or wait for a GPIB event to occur.  The mask argument
 * is a bit vector corresponding to the status bit vector.  It
 * has a bit set for each condition which can terminate the wait
 * If the mask is 0 then
 * no condition is waited for.
 */
int ibwait( gpib_board_t *board, unsigned int *mask, unsigned int pad, int sad )
{
	int retval = 0;
	gpib_device_t *device;

	if( *mask == 0 )
	{
		return 0;
	}
	else if( *mask & ~WAITBITS )
	{
		printk( "bad mask 0x%x \n", *mask );
		return -1;
	}

	device = get_gpib_device( board, pad, sad );
	if( device == NULL )
		return -EINVAL;

	osStartTimer( board, board->usec_timeout );

	if( wait_event_interruptible( board->wait, wait_satisfied( board, device, *mask ) ) )
	{
		printk( "wait interrupted\n" );
		retval = -ERESTARTSYS;
	}

	osRemoveTimer( board );

	*mask = ibstatus( board );
	if( num_status_bytes( device ) ) *mask |= RQS;

	return retval;
}

