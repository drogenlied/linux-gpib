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
#include <unistd.h>
#include <string.h>
#include <mcheck.h>

#include "gpib/ib.h"

struct board_descriptors
{
	int master;
	int slave;
};

static int init_board( int board_desc )
{
	int status;

	ibtmo( board_desc, T3s );
	if( ThreadIbsta() & ERR )
	{
		fprintf( stderr, "FAILED: ibtmo() error in %s\n", __FUNCTION__ );
		return -1;
	}

	ibeot( board_desc, 1 );
	if( ibsta & ERR )
	{
		fprintf( stderr, "FAILED: ibeot() error in %s\n", __FUNCTION__ );
		return -1;
	}

	status = ibeos( board_desc, 0 );
	if( status & ERR )
	{
		fprintf( stderr, "FAILED: ibtmo() error in %s\n", __FUNCTION__ );
		return -1;
	}
	if( status != ThreadIbsta() || ThreadIbsta() != ibsta )
	{
		fprintf( stderr, "FAILED: status bits inconsistent in %s\n", __FUNCTION__ );
		return -1;
	}

	return 0;
}

static int find_boards( struct board_descriptors *boards )
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
	}
	if( init_board( boards->master ) )
	{
		fprintf( stderr, "error initializing master\n" );
		return -1;
	}
	if( init_board( boards->slave ) )
	{
		fprintf( stderr, "error initializing slave\n" );
		return -1;
	}

	fprintf( stderr, "OK\n" );
	return 0;
}

static int open_slave_device_descriptor( const struct board_descriptors *boards,
	int timeout, int eot, int eos )
{
	int pad, sad;
	int ud;
	int status;

	status = ibask( boards->slave, IbaPAD, &pad );
	if( status & ERR )
	{
		fprintf( stderr, "FAILED: could not query slave device pad\n" );
		return -1;
	}
	status = ibask( boards->slave, IbaSAD, &sad );
	if( status & ERR )
	{
		fprintf( stderr, "FAILED: could not query slave device sad\n" );
		return -1;
	}
	ud = ibdev( boards->master, pad, sad, timeout, eot, eos );
	if( ud < 0 )
	{
		fprintf( stderr, "FAILED: could not open slave device descriptor\n" );
	}
	return ud;
}

static const char read_write_string1[] = "dig8esdfas sdf\n";
static const char read_write_string2[] = "DFLIJFES8F3	";

struct read_write_slave_parameters
{
	int slave_board;
	int retval;
};

static void* read_write_slave_thread( void *arg )
{
	volatile struct read_write_slave_parameters *param = arg;
	char buffer[ 1000 ];
	int status;
	int i;

	for( i = 0; i < 2; i++ )
	{
		memset( buffer, 0, sizeof( buffer ) );
		status = ibrd( param->slave_board, buffer, sizeof( buffer ) );
		if( status & ERR )
		{
			fprintf( stderr, "FAILED: slave thread got error from ibrd\n" );
			param->retval = -1;
			return NULL;
		}
		if( strcmp( buffer, read_write_string1 ) )
		{
			fprintf( stderr, "FAILED: slave thread got bad data from ibrd\n" );
			param->retval = -1;
			return NULL;
		}
		status = ibwrt( param->slave_board, read_write_string2, strlen( read_write_string2 ) + 1 );
		if( status & ERR )
		{
			fprintf( stderr, "FAILED: slave thread got error from ibwrt\n" );
			param->retval = -1;
			return NULL;
		}
	}
	param->retval = 0;
	return NULL;
}

static int read_write_test( const struct board_descriptors *boards )
{
	int ud;
	pthread_t slave_thread;
	volatile struct read_write_slave_parameters param;
	int status;
	char buffer[ 1000 ];
	int i;

	fprintf( stderr, "%s...", __FUNCTION__ );
	ud = open_slave_device_descriptor( boards, T3s, 1, 0 );
	if( ud < 0 )
		return -1;
	param.slave_board = boards->slave;
	if( pthread_create( &slave_thread, NULL, read_write_slave_thread, (void*)&param ) )
	{
		fprintf( stderr, "FAILED: error creating slave thread\n" );
		ibonl( ud, 0 );
		return -1;
	}
	for( i = 0; i < 2; i++ )
	{
		status = ibwrt( ud, read_write_string1, strlen( read_write_string1 ) + 1 );
		if( ( status & ERR ) || !( status & CMPL ) )
		{
			fprintf( stderr, "FAILED: write status 0x%x, error %i\n", ThreadIbsta(),
				ThreadIberr() );
			pthread_join( slave_thread, NULL );
			ibonl( ud, 0 );
			return -1;
		}
		memset( buffer, 0, sizeof( buffer ) );
		status = ibrd( ud, buffer, sizeof( buffer ) );
		if( ( status & ERR ) || !( status & CMPL ) || !( status & END ) )
		{
			fprintf( stderr, "FAILED: read status 0x%x, error %i\n", ThreadIbsta(),
				ThreadIberr() );
			pthread_join( slave_thread, NULL );
			ibonl( ud, 0 );
			return -1;
		}
		if( strcmp( buffer, read_write_string2 ) )
		{
			fprintf( stderr, "FAILED: got bad data from ibrd\n" );
			fprintf( stderr, "received %i bytes:%s\n", ThreadIbcnt(), buffer );
			pthread_join( slave_thread, NULL );
			ibonl( ud, 0 );
			return -1;
		}
	}
	if( pthread_join( slave_thread, NULL ) )
	{
		fprintf( stderr, "FAILED: error joining slave thread\n" );
		ibonl( ud, 0 );
		return -1;
	}
	if( param.retval < 0 )
	{
		ibonl( ud, 0 );
		return -1;
	}
	fprintf( stderr, "OK\n" );
	ibonl( ud, 0 );
	return 0;
}

static int async_read_write_test( const struct board_descriptors *boards )
{
	int ud;
	char buffer[ 1000 ];
	int i;
	int status;

	fprintf( stderr, "%s...", __FUNCTION__ );
	ud = open_slave_device_descriptor( boards, T3s, 1, 0 );
	if( ud < 0 )
		return -1;
	for( i = 0; i < 2; i++ )
	{
		status = ibwrta( ud, read_write_string1, strlen( read_write_string1 ) + 1 );
		if( status & ERR )
		{
			fprintf( stderr, "FAILED: write error %i\n", ThreadIberr() );
			ibonl( ud, 0 );
			return -1;
		}
		memset( buffer, 0, sizeof( buffer ) );
		status = ibrda( boards->slave, buffer, sizeof( buffer ) );
		if( status & ERR )
		{
			fprintf( stderr, "FAILED: read error %i\n", ThreadIberr() );
			ibonl( ud, 0 );
			return -1;
		}
		status = ibwait( ud, CMPL | TIMO );
		if( ( status & ERR ) || !( status & CMPL ) )
		{
			fprintf( stderr, "FAILED: write status 0x%x, error %i\n", ThreadIbsta(),
				ThreadIberr() );
			ibonl( ud, 0 );
			return -1;
		}
		status = ibwait( boards->slave, CMPL | TIMO );
		if( ( status & ERR ) || !( status & CMPL ) )
		{
			fprintf( stderr, "FAILED: write status 0x%x, error %i\n", ThreadIbsta(),
				ThreadIberr() );
			ibonl( ud, 0 );
			return -1;
		}
		if( strcmp( buffer, read_write_string1 ) )
		{
			fprintf( stderr, "FAILED: got bad data from ibrd\n" );
			fprintf( stderr, "received %i bytes:%s\n", ThreadIbcnt(), buffer );
			ibonl( ud, 0 );
			return -1;
		}
	}
	ibonl( ud, 0 );
	fprintf( stderr, "OK\n" );
	return 0;
}

static int serial_poll_test( const struct board_descriptors *boards )
{
	int ud;
	char result;
	const int status_byte = 0x43;

	fprintf( stderr, "%s...", __FUNCTION__ );
	ud = open_slave_device_descriptor( boards, T3s, 1, 0 );
	if( ud < 0 )
		return -1;

	/* make sure status queue is empty */
	while( ibwait( ud, 0 ) & RQS )
	{
		ibrsp( ud, &result );
		if( ThreadIbsta() & ERR )
		{
			fprintf( stderr, "FAILED: error emptying status queue\n" );
			ibonl( ud, 0 );
			return -1;
		}
	}

	if( ibconfig( boards->master, IbcAUTOPOLL, 1 ) & ERR )
	{
		fprintf( stderr, "FAILED: failed to enable autopolling\n" );
		ibonl( ud, 0 );
		return -1;
	}

	ibrsv( boards->slave, status_byte );
	if( ThreadIbsta() & ERR )
	{
		fprintf( stderr, "FAILED: failed to request service\n" );
		ibonl( ud, 0 );
		return -1;
	}

	ibwait( ud, RQS | TIMO );
	if( ( ThreadIbsta() & ERR ) || ( ThreadIbsta() & TIMO ) || !( ThreadIbsta() & RQS ) )
	{
		fprintf( stderr, "FAILED: did not receive service request\n" );
		ibonl( ud, 0 );
		return -1;
	}
	result = 0;
	ibrsp( ud, &result );
	if( ThreadIbsta() & ERR )
	{
		fprintf( stderr, "FAILED: failed to read status byte\n" );
		ibonl( ud, 0 );
		return -1;
	}

	if( ( result & 0xff ) != status_byte )
	{
		fprintf( stderr, "FAILED: status byte value is incorrect\n" );
		ibonl( ud, 0 );
		return -1;
	}

	ibonl( ud, 0 );
	if( ThreadIbsta() & ERR )
	{
		fprintf( stderr, "FAILED: failed to take descriptor offline\n" );
		return -1;
	}

	fprintf( stderr, "OK\n" );

	return 0;
}

int main( int argc, char *argv[] )
{
	struct board_descriptors boards;
	int retval;

	if( mcheck( 0 ) )
		fprintf( stderr, "mcheck() failed!\n" );

	retval = find_boards( &boards );
	if( retval < 0 ) return retval;

	read_write_test( &boards );

	async_read_write_test( &boards );
	serial_poll_test( &boards );

	return 0;
}

