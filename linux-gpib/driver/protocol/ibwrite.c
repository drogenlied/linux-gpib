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
 * operation terminates only on I/O complete. 
 *
 * NOTE:
 *      1.  Prior to beginning the write, the interface is
 *          placed in the controller standby state.
 *      2.  Prior to calling ibwrt, the intended devices as
 *          well as the interface board itself must be
 *          addressed by calling ibcmd.
 */
ssize_t ibwrt(gpib_device_t *device, uint8_t *buf, size_t cnt, int send_eoi)
{
	size_t bytes_sent = 0;
	ssize_t ret = 0;

	if(cnt == 0) return 0;

	device->interface->go_to_standby(device);
	// mark io in progress
	clear_bit(CMPL_NUM, &device->status);
	osStartTimer(device, timeidx);

	ret = device->interface->write(device, buf, cnt, send_eoi);
	if(ret < 0)
	{
		printk("gpib write error\n");
	}else
	{
		buf += ret;
		bytes_sent += ret;
	}

	if(ibstatus(device) & TIMO)
		ret = -ETIMEDOUT;

	osRemoveTimer(device);

	// mark io complete
	set_bit(CMPL_NUM, &device->status);

	if(ret < 0) return ret;
	
	return bytes_sent;
}
