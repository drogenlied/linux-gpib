/***************************************************************************
                              ibread.c
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

static int gpib_clear_to_read( gpib_board_t *board )
{
	unsigned int status;

	status = ibstatus( board );
	if( ( status & ATN ) == 0 && ( status & LACS ) ) return 1;

	return 0;
}

/*
 * IBRD
 * Read up to 'length' bytes of data from the GPIB into buf.  End
 * on detection of END (EOI and or EOS) and set 'end_flag'.
 *
 * NOTE:
 *      1.  The interface is placed in the controller standby
 *          state prior to beginning the read.
 *      2.  Prior to calling ibrd, the intended devices as well
 *          as the interface board itself must be addressed by
 *          calling ibcmd.
 */

ssize_t ibrd( gpib_board_t *board, uint8_t *buf, size_t length, int *end_flag )
{
	size_t count = 0;
	ssize_t ret = 0;
	int retval;

	if( length == 0 )
	{
		printk( "gpib: ibrd() called with zero length?\n");
		return 0;
	}

	if( board->master )
	{
		retval = ibgts( board );
		if( retval < 0 ) return retval;
	}
	/* XXX reseting timer here could cause timeouts take longer than they should,
	 * since read_ioctl calls this
	 * function in a loop, there is probably a similar problem with writes/commands */
	osStartTimer( board, board->usec_timeout );
	// initialize status to END not yet received
	clear_bit(END_NUM, &board->status);

	/* wait until board is addressed as listener and ATN is not asserted
	 * (doesn't really matter) */
	if( wait_event_interruptible( board->wait,
		gpib_clear_to_read( board ) ||
		test_bit( TIMO_NUM, &board->status ) ) )
	{
		ret = -ERESTARTSYS;
		printk( "gpib: wait interrupted while waiting to be addressed as listener\n");
	}else
	{
		do
		{
			ret = board->interface->read(board, buf, length - count, end_flag);
			if(ret < 0)
			{
				printk("gpib read error\n");
			}else
			{
				buf += ret;
				count += ret;
			}
		}while(ret > 0 && count < length && end_flag == 0);
	}

	osRemoveTimer(board);

	return (ret < 0) ? ret : count;
}

