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

int find_eos( uint8_t *buffer, size_t length, int eos, int eos_flags )
{
	unsigned int i;
	unsigned int compare_mask;

	if( eos_flags & BIN ) compare_mask = 0xff;
	else compare_mask = 0x7f;

	for( i = 0; i < length; i++ )
	{
		if( ( buffer[i] & compare_mask ) == ( eos & compare_mask ) )
		return i;
	}

	return -1;
}

ssize_t my_ibwrt( ibBoard_t *board, ibConf_t *conf,
	uint8_t *buffer, size_t count )
{
	read_write_ioctl_t write_cmd;
	size_t block_size;
	size_t bytes_sent = 0;
	int retval;

	iblcleos( conf );

	if( conf->is_interface == 0 )
	{
		// set up addressing
		if( send_setup( conf ) < 0 )
		{
			return -1;
		}
	}

	set_timeout( board, conf->usec_timeout );

	while( count )
	{
		int eos_on_eoi;
		int eos_found = 0;

		eos_on_eoi = conf->eos_flags & XEOS;

		block_size = count;

		if( eos_on_eoi )
		{
			retval = find_eos( buffer, count, conf->eos, conf->eos_flags );
			if( retval < 0 ) eos_found = 0;
			else
			{
				block_size = retval;
				eos_found = 1;
			}
		}

		write_cmd.buffer = buffer;
		write_cmd.count = block_size;
		write_cmd.end = conf->send_eoi ||
			( eos_on_eoi && eos_found );

		if( ioctl( board->fileno, IBWRT, &write_cmd) < 0)
		{
			return -1;
		}
		count -= block_size;
		bytes_sent += block_size;
		buffer += block_size;
	}

	return bytes_sent;
}

int ibwrt(int ud, void *rd, long cnt)
{
	ibConf_t *conf;
	ibBoard_t *board;
	ssize_t count;

	conf = enter_library( ud );
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
				conf->timed_out = 1;
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




