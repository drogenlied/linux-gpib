/***************************************************************************
                          lib/ibWrt.c
                             -------------------

	copyright            : (C) 2001,2002 by Frank Mori Hess
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

#include "ib_internal.h"
#include "ibP.h"
#include <sys/ioctl.h>

ssize_t my_ibwrt( const ibBoard_t *board, const ibConf_t *conf,
	uint8_t *buffer, size_t count )
{
	read_write_ioctl_t write_cmd;

	iblcleos( conf );

	if( conf->is_interface == 0 )
	{
		// set up addressing
		if( send_setup( board, conf ) < 0 )
		{
			return -1;
		}
	}

	write_cmd.buffer = buffer;
	write_cmd.count = count;
	write_cmd.end = conf->send_eoi;

	set_timeout( board, conf->usec_timeout );

	if( ioctl( board->fileno, IBWRT, &write_cmd) < 0)
	{
		return -1;
	}
	return write_cmd.count;
}

int ibwrt(int ud, void *rd, long cnt)
{
	ibConf_t *conf;
	ibBoard_t *board;
	ssize_t count;

	conf = enter_library( ud, 1 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	conf->end = 0;

	board = interfaceBoard( conf );

	count = my_ibwrt( board, conf, rd, cnt );

	if( count < 0 )
	{
		switch( errno )
		{
			case ETIMEDOUT:
				setIberr( EABO );
				board->timed_out = 1;
				break;
			default:
				break;
		}
		return exit_library( ud, 1 );
	}

	if(count != cnt)
	{
		setIberr( EDVR );
		return exit_library( ud, 1 );
	}

	if( conf->send_eoi )
		conf->end = 1;
	setIbcnt( count );

	return exit_library( ud, 0 );
}




