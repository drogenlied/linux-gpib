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
 */
IBLCL ssize_t ibwrt(uint8_t *buf, size_t cnt, int more)
{
	size_t bytes_sent = 0;
	ssize_t ret = 0;
	int status = ibstatus();

	if((status & TACS) == 0)
	{
		printk("gpib: not talker during write\n");
		return -1;
	}

	if(cnt == 0 && more == 0)
	{
		printk("gpib: ibwrt called with no data\n");
		return -1;
	}
	driver->go_to_standby(driver);
	// mark io in progress
	clear_bit(CMPL_NUM, &driver->status);
	osStartTimer(timeidx);
	while ((bytes_sent < cnt) && !(ibstatus() & (TIMO)))
	{
		ret = driver->write(driver, buf, cnt, !more);
		if(ret < 0)
		{
			printk("gpib write error\n");
			break;
		}
		buf += ret;
		bytes_sent += ret;
	}

	if(ibstatus() & TIMO)
		ret = -ETIMEDOUT;

	osRemoveTimer();

	// mark io complete
	set_bit(CMPL_NUM, &driver->status);

	if(ret < 0) return ret;
	
	return bytes_sent;
}
