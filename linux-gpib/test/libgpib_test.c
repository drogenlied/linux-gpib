/***************************************************************************
                             libgpib_test.c
                             -------------------

Test program for libgpib.  Requires two gpib boards installed in the
computer, on the same GPIB bus, and one of which is the system controller.

    copyright            : (C) 2003 by Frank Mori Hess
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

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>

#include "gpib/ib.h"

struct board_descriptors
{
	int master;
	int slave;
};

int find_boards( struct board_descriptors *boards )
{
	int i;
	int status, result;

	boards->master = -1;
	boards->slave = -1;
	fprintf( stderr, "Finding boards..." );
	for( i = 0; i < GPIB_MAX_NUM_BOARDS; i++ )
	{
		status = ibask( i, IbaSC, &result );
		if( status & ERR )
			continue;
		if( boards->master < 0 && result != 0 )
		{
			boards->master = i;
		}else if( boards->slave < 0 && result == 0 )
		{
			boards->slave = i;
		}
		if( boards->master >= 0 && boards->slave >= 0 ) break;
	}
	if( boards->master < 0 )
	{
		fprintf( stderr, "FAILED: could not find system controller board\n" );
		return -1;
	}else if( boards->slave < 0 )
	{
		fprintf( stderr, "FAILED: could not find slave board\n" );
		return -1;
	}else
	{
		fprintf( stderr, "OK\n" );
		return 0;
	}
}

const char *read_write_string1 = "dig8esdfas sdf\n";
const char *read_write_string2 = "DFLIJFES8F3	";

struct read_write_slave_parameters
{
	int slave_board;
	int retval;
};

void* read_write_slave_thread( void *arg )
{
	volatile struct read_write_slave_parameters *param = arg;
	char buffer[ 1000 ];
	int status;

	status = ibrd( param->slave_board, buffer, sizeof( buffer ) - 1 );
	if( status & ERR )
	{
		fprintf( stderr, "FAILED: slave thread got error from ibrd\n" );
		param->retval = -1;
		return NULL;
	}
	buffer[ ThreadIbcntl() ] = 0;
	if( strcmp( buffer, read_write_string1 ) )
	{
		fprintf( stderr, "FAILED: slave thread got bad data from ibrd\n" );
		param->retval = -1;
		return NULL;
	}
	param->retval = 0;
	return NULL;
}

int read_write_test( const struct board_descriptors *boards )
{
	int ud;
	pthread_t slave_thread;
	volatile struct read_write_slave_parameters param;
	int status;
	int pad, sad;

	fprintf( stderr, "read/write test..." );
	status = ibask( boards->slave, IbaPAD, &pad );
	if( status & ERR )
	{
		fprintf( stderr, "FAILED: could not query slave pad\n" );
		return -1;
	}
	status = ibask( boards->slave, IbaSAD, &sad );
	if( status & ERR )
	{
		fprintf( stderr, "FAILED: could not query slave sad\n" );
		return -1;
	}
	ud = ibdev( boards->master, pad, sad, T1s, 1, 0 );
	if( ud < 0 )
	{
		fprintf( stderr, "FAILED: could not open device descriptor\n" );
		return -1;
	}
	param.slave_board = boards->slave;
	if( pthread_create( &slave_thread, NULL, read_write_slave_thread, (void*)&param ) )
	{
		fprintf( stderr, "FAILED: error creating slave thread\n" );
		return -1;
	}
	status = ibwrt( ud, read_write_string1, strlen( read_write_string1 ) );
	if( status & ERR )
	{
		fprintf( stderr, "FAILED: write error %i\n", ThreadIberr() );
		pthread_join( slave_thread, NULL );
		return -1;
	}
	if( pthread_join( slave_thread, NULL ) )
	{
		fprintf( stderr, "FAILED: error joining slave thread\n" );
		return -1;
	}
	if( param.retval < 0 )
	{
		return -1;
	}
	fprintf( stderr, "OK\n" );
	return 0;
}

int main( int argc, char *argv[] )
{
	struct board_descriptors boards;
	int retval;

	retval = find_boards( &boards );
	if( retval < 0 ) return retval;

	retval = read_write_test( &boards );
	if( retval < 0 ) return retval;

	return 0;
}

