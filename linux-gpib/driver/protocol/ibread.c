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


#include <ibprot.h>


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

ssize_t ibrd(gpib_board_t *board, uint8_t *buf, size_t length, int *end_flag)
{
	size_t count = 0;
	ssize_t ret = 0;

	if(length == 0) return 0;

	board->interface->go_to_standby(board);
	osStartTimer(board, timeidx);
	// mark io in progress
	clear_bit(CMPL_NUM, &board->status);
	// initialize status to END not yet received
	clear_bit(END_NUM, &board->status);

	ret = board->interface->read(board, buf, length, end_flag);
	if(ret < 0)
	{
		printk("gpib read error\n");
	}else
	{
		buf += ret;
		count += ret;
	}

	osRemoveTimer(board);
	// mark io completed
	set_bit(CMPL_NUM, &board->status);

	return ret ? ret : count;
}

