/***************************************************************************
                                 ibdev.c
                             -------------------
    begin                : Tues Feb 12 2002
    copyright            : (C) 2002 by Frank Mori Hess
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
#include <stdlib.h>

static int is_device_addr( int minor, int pad, int sad )
{
	ibBoard_t *board;

	board = &ibBoard[ minor ];

	if( gpib_address_equal( board->pad, board->sad, pad, sad ) == 0 )
	{
		return 1;
	}

	return 0;
}

int ibdev( int minor, int pad, int sad, int timo, int eot, int eosmode )
{
	int retval;

	retval = ibParseConfigFile();
	if(retval < 0)
	{
		setIbsta( ERR );
		return -1;
	}

	sad -= sad_offset;

	if( is_device_addr( minor, pad, sad ) == 0 )
	{
		setIberr( EARG );
		setIbsta( ERR );
		fprintf( stderr, "libgpib: ibdev gpib address already in use by\n"
			"\tinterface board.  Use board index or ibfind() to open boards.\n" );
		return -1;
	}

	return my_ibdev( minor, pad, sad, timeout_to_usec( timo ),
		eot, eosmode & 0xff, eosmode & 0xff00 );
}

int my_ibdev( int minor, int pad, int sad, unsigned int usec_timeout, int send_eoi, int eos, int eos_flags)
{
	int uDesc;
	ibConf_t new_conf;
	ibConf_t *conf;
	ibBoard_t *board;

	init_ibconf( &new_conf );
	new_conf.settings.pad = pad;
	new_conf.settings.sad = sad;                        /* device address                   */
	new_conf.settings.board = minor;                         /* board number                     */
	new_conf.settings.eos = eos;                           /* local eos modes                  */
	new_conf.settings.eos_flags = eos_flags;
	new_conf.settings.usec_timeout = usec_timeout;
	if( send_eoi )
		new_conf.settings.send_eoi = 1;
	else
		new_conf.settings.send_eoi = 0;
	new_conf.defaults = new_conf.settings;
	
	// check if it is an interface board
	board = &ibBoard[ minor ];
	if( gpib_address_equal( board->pad, board->sad, new_conf.settings.pad, new_conf.settings.sad ) )
	{
		new_conf.is_interface = 1;
	}else
		new_conf.is_interface = 0;

	uDesc = ibGetDescriptor(new_conf);
	if(uDesc < 0)
	{
		fprintf(stderr, "libgpib: ibdev failed to get descriptor\n");
		setIbsta( ERR );
		return -1;
	}

	conf = enter_library( uDesc );
	if( conf == NULL )
	{
		exit_library( uDesc, 1 );
		return -1;
	}
	// XXX do local lockout if appropriate

	exit_library( uDesc, 0 );
	return uDesc;
}
