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
#include <ibP.h>
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

int ibdev(int minor, int pad, int sad, int timo, int eot, int eos)
{
	if( is_device_addr( minor, pad, sad ) == 0 )
	{
		setIberr( EARG );
		setIbsta( ERR );
		fprintf( stderr, "libgpib: ibdev gpib address already in use by\n"
			"\tinterface board.  Use ibfind() to open boards.\n" );
		return -1;
	}

	return my_ibdev( minor, pad, sad, timeout_to_usec( timo ),
		eot, eos & 0xff, ( eos >> 8 ) & 0xff );
}

int my_ibdev( int minor, int pad, int sad, unsigned int usec_timeout, int send_eoi, int eos, int eos_flags)
{
	char *envptr;
	int retval;
	int uDesc;
	ibConf_t conf;
	ibBoard_t *board;

	/* load config */

	envptr = getenv("IB_CONFIG");
	if(envptr)
		retval = ibParseConfigFile(envptr);
	else
		retval = ibParseConfigFile(DEFAULT_CONFIG_FILE);
	if(retval < 0)
	{
		setIbsta( ERR );
		return -1;
	}

	init_ibconf( &conf );
	conf.pad = pad;
	conf.sad = sad - sad_offset;                        /* device address                   */
	conf.board = minor;                         /* board number                     */
	conf.eos = eos;                           /* local eos modes                  */
	conf.eos_flags = eos_flags;
	conf.usec_timeout = usec_timeout;
	if( send_eoi )
		conf.send_eoi = 1;
	else
		conf.send_eoi = 0;
	// check if it is an interface board
	board = &ibBoard[ minor ];
	if( gpib_address_equal( board->pad, board->sad, conf.pad, conf.sad ) )
	{
		conf.is_interface = 1;
	}else
		conf.is_interface = 0;

	uDesc = ibGetDescriptor(conf);
	if(uDesc < 0)
	{
		fprintf(stderr, "libgpib: ibdev failed to get descriptor\n");
		setIbsta( ERR );
		return -1;
	}

	// XXX do local lockout if appropriate

	return uDesc;
}
