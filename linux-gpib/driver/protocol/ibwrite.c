/***************************************************************************
                              ibwrite.c
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
 * IBWRT
 * Write cnt bytes of data from buf to the GPIB.  The write
 * operation terminates only on I/O complete.  By default,
 * EOI is always sent along with the last byte.  'more' is
 * a boolean value indicating that there remains more data
 * that will be written as a part of this output (so don't
 * send EOI/EOS).
 *
 * NOTE:
 *      1.  Prior to beginning the write, the interface is
 *          placed in the controller standby state.
 *      2.  Prior to calling ibwrt, the intended devices as
 *          well as the interface board itself must be
 *          addressed by calling ibcmd.
 *      3.  Be sure to type cast the buffer to (faddr_t) before
 *          calling this function.
 */
IBLCL int ibwrt(uint8_t *buf, size_t cnt, unsigned int more)
{
	unsigned int	requested_cnt;
	ssize_t ret;
	int status = board.update_status();

	DBGin("ibwrt");
	if((status & TACS) == 0) 
	{
		ibcnt = 0;
		DBGout();
		return status;
	}
	board.go_to_standby();
	// mark io in progress
	clear_bit(CMPL_NUM, &board.status);
	osStartTimer(timeidx);
	requested_cnt = cnt;
	while ((cnt > 0) && !(board.status & (ERR | TIMO))) {
		ret = board.write(buf, cnt, !more);
		if(ret < 0)
		{
			printk("gpib write error\n");
			break;
		}
		buf += ret;
		cnt -= ret;
	}
	ibcnt = requested_cnt - cnt;
	osRemoveTimer();
	
	// mark io complete
	set_bit(CMPL_NUM, &board.status);

	DBGout();
	return board.update_status();
}
