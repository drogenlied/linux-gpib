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

#define PRINT_FAILED() \
	fprintf( stderr, "FAILED: %s line %i, ibsta 0x%x, iberr %i, ibcntl %li\n", \
		__FILE__, __LINE__, ThreadIbsta(), ThreadIberr(), ThreadIbcntl() ); \

static int init_board( int board_desc )
{
	int status;

	ibtmo( board_desc, T3s );
	if( ThreadIbsta() & ERR )
	{
		PRINT_FAILED();
		return -1;
	}

	ibeot( board_desc, 1 );
	if( ibsta & ERR )
	{
		PRINT_FAILED();
		return -1;
	}

	status = ibeos( board_desc, 0 );
	if( status & ERR )
	{
		PRINT_FAILED();
		return -1;
	}
	if( status != ThreadIbsta() || ThreadIbsta() != ibsta )
	{
		PRINT_FAILED();
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
		PRINT_FAILED();
		return -1;
	}else if( boards->slave < 0 )
	{
		PRINT_FAILED();
		return -1;
	}
	if( init_board( boards->master ) )
	{
		PRINT_FAILED();
		return -1;
	}
	if( init_board( boards->slave ) )
	{
		PRINT_FAILED();
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
		PRINT_FAILED();
		return -1;
	}
	status = ibask( boards->slave, IbaSAD, &sad );
	if( status & ERR )
	{
		PRINT_FAILED();
		return -1;
	}
	ud = ibdev( boards->master, pad, sad, timeout, eot, eos );
	if( ud < 0 )
	{
		PRINT_FAILED();
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
			PRINT_FAILED();
			param->retval = -1;
			return NULL;
		}
		if( strcmp( buffer, read_write_string1 ) )
		{
			PRINT_FAILED();
			fprintf( stderr, "got bad data from ibrd\n" );
			fprintf( stderr, "received %i bytes:%s\n", ThreadIbcnt(), buffer );
			param->retval = -1;
			return NULL;
		}
		status = ibwrt( param->slave_board, read_write_string2, strlen( read_write_string2 ) + 1 );
		if( status & ERR )
		{
			PRINT_FAILED();
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
		PRINT_FAILED();
		ibonl( ud, 0 );
		return -1;
	}
	for( i = 0; i < 2; i++ )
	{
		status = ibwrt( ud, read_write_string1, strlen( read_write_string1 ) + 1 );
		if( ( status & ERR ) || !( status & CMPL ) )
		{
			PRINT_FAILED();
			pthread_join( slave_thread, NULL );
			ibonl( ud, 0 );
			return -1;
		}
		memset( buffer, 0, sizeof( buffer ) );
		status = ibrd( ud, buffer, sizeof( buffer ) );
		if( ( status & ERR ) || !( status & CMPL ) || !( status & END ) )
		{
			PRINT_FAILED();
			pthread_join( slave_thread, NULL );
			ibonl( ud, 0 );
			return -1;
		}
		if( strcmp( buffer, read_write_string2 ) )
		{
			PRINT_FAILED();
			fprintf( stderr, "received bytes:%s\n", buffer );
			pthread_join( slave_thread, NULL );
			ibonl( ud, 0 );
			return -1;
		}
	}
	if( pthread_join( slave_thread, NULL ) )
	{
		PRINT_FAILED();
		ibonl( ud, 0 );
		return -1;
	}
	if( param.retval < 0 )
	{
		PRINT_FAILED();
		ibonl( ud, 0 );
		return -1;
	}
	ibonl( ud, 0 );
	if( ThreadIbsta() & ERR )
	{
		PRINT_FAILED();
		return -1;
	}
	fprintf( stderr, "OK\n" );
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
			PRINT_FAILED();
			ibonl( ud, 0 );
			return -1;
		}
		memset( buffer, 0, sizeof( buffer ) );
		status = ibrda( boards->slave, buffer, sizeof( buffer ) );
		if( status & ERR )
		{
			PRINT_FAILED();
			fprintf( stderr, "read error %i\n", ThreadIberr() );
			ibonl( ud, 0 );
			return -1;
		}
		status = ibwait(ud, CMPL | TIMO);
		if((status & (ERR | TIMO)) || !(status & CMPL) )
		{
			PRINT_FAILED();
			fprintf( stderr, "write status 0x%x, error %i\n", ThreadIbsta(),
				ThreadIberr() );
			ibonl( ud, 0 );
			return -1;
		}
		status = ibwait( boards->slave, CMPL | TIMO );
		if((status & (ERR | TIMO)) || !(status & CMPL))
		{
			PRINT_FAILED();
			fprintf( stderr, "write status 0x%x, error %i\n", ThreadIbsta(),
				ThreadIberr() );
			ibonl( ud, 0 );
			return -1;
		}
		if( strcmp( buffer, read_write_string1 ) )
		{
			PRINT_FAILED();
			fprintf( stderr, "got bad data from ibrd\n" );
			fprintf( stderr, "received %i bytes:%s\n", ThreadIbcnt(), buffer );
			ibonl( ud, 0 );
			return -1;
		}
	}
	ibonl( ud, 0 );
	if( ThreadIbsta() & ERR )
	{
		PRINT_FAILED();
		return -1;
	}
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
			PRINT_FAILED();
			ibonl( ud, 0 );
			return -1;
		}
	}

	if( ibconfig( boards->master, IbcAUTOPOLL, 1 ) & ERR )
	{
		PRINT_FAILED();
		ibonl( ud, 0 );
		return -1;
	}

	ibrsv( boards->slave, status_byte );
	if( ThreadIbsta() & ERR )
	{
		PRINT_FAILED();
		ibonl( ud, 0 );
		return -1;
	}

	ibwait( ud, RQS | TIMO );
	if((ThreadIbsta() & (ERR | TIMO)) || !(ThreadIbsta() & RQS))
	{
		PRINT_FAILED();
		ibonl( ud, 0 );
		return -1;
	}
	result = 0;
	ibrsp( ud, &result );
	if( ThreadIbsta() & ERR )
	{
		PRINT_FAILED();
		ibonl( ud, 0 );
		return -1;
	}

	if( ( result & 0xff ) != status_byte )
	{
		PRINT_FAILED();
		ibonl( ud, 0 );
		return -1;
	}

	ibonl( ud, 0 );
	if( ThreadIbsta() & ERR )
	{
		PRINT_FAILED();
		return -1;
	}

	fprintf( stderr, "OK\n" );

	return 0;
}

static int parallel_poll_test( const struct board_descriptors *boards )
{
	int ud;
	char result;
	int line, sense;
	int ist;

	fprintf( stderr, "%s...", __FUNCTION__ );
	ud = open_slave_device_descriptor( boards, T3s, 1, 0 );
	if( ud < 0 )
		return -1;

	ist = 1;
	ibist( boards->slave, ist );

	line = 2; sense = 1;
	ibppc( ud, PPE_byte( line, sense ) );
	if( ibsta & ERR )
	{
		PRINT_FAILED();
		return -1;
	}
	ibrpp( boards->master, &result );
	if( ThreadIbsta() & ERR )
	{
		PRINT_FAILED();
		return -1;
	}
	if( ( result & ( 1 << ( line - 1 ) ) ) == 0 )
	{
		PRINT_FAILED();
		fprintf( stderr, "parallel poll result: 0x%x\n", (unsigned int)result );
		return -1;
	}

	ibonl( ud, 0 );
	if( ThreadIbsta() & ERR )
	{
		PRINT_FAILED();
		return -1;
	}

	fprintf( stderr, "OK\n" );
	return 0;
}

static int do_eos_pass(const struct board_descriptors *boards,
	int eosmode, const char *test_message, const char *first_read_result,
	const char *second_read_result)
{
	int ud;
	char buffer[1024];
	int status;

	ud = open_slave_device_descriptor( boards, T3s, 0, eosmode );
	if( ud < 0 )
		return -1;
	ibwrta(boards->slave, test_message, strlen(test_message));
	if( ThreadIbsta() & ERR )
	{
		PRINT_FAILED();
		return -1;
	}
	ibrd(ud, buffer, sizeof(buffer) - 1);
	if( ThreadIbsta() & ERR )
	{
		PRINT_FAILED();
		return -1;
	}
	buffer[ThreadIbcntl()] = '\0';
	if(strcmp(buffer, first_read_result))
	{
		PRINT_FAILED();
		fprintf(stderr, "first read got: '%s'\n"
			"expected: '%s'\n", buffer, first_read_result);
		return -1;
	}
	if(second_read_result != NULL)
	{
		ibrd(ud, buffer, sizeof(buffer) - 1);
		if( ThreadIbsta() & ERR )
		{
			PRINT_FAILED();
			return -1;
		}
		buffer[ThreadIbcntl()] = '\0';
		if(strcmp(buffer, second_read_result))
		{
			PRINT_FAILED();
			fprintf(stderr, "second read got: '%s'\n"
				"expected: '%s'\n", buffer, second_read_result);
			return -1;
		}
	}
	status = ibwait(boards->slave, CMPL | TIMO);
	if((status & (ERR | TIMO)) || !(status & CMPL))
	{
		PRINT_FAILED();
		fprintf( stderr, "write status 0x%x, error %i\n", ThreadIbsta(),
			ThreadIberr());
		return -1;
	}
	ibonl( ud, 0 );
	if( ThreadIbsta() & ERR )
	{
		PRINT_FAILED();
		return -1;
	}
	return 0;
}

static int eos_test( const struct board_descriptors *boards )
{
	int retval;
	fprintf( stderr, "%s...", __FUNCTION__ );

	retval = do_eos_pass(boards, 'x' | REOS, "adfis\xf8gibblex",
		"adfis\xf8", "gibblex");
	if(retval < 0) return retval;

	retval = do_eos_pass(boards, 'x' | REOS | BIN, "adfis\xf8gibblex",
		"adfis\xf8gibblex", NULL);
	if(retval < 0) return retval;

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

	retval = read_write_test( &boards );
	if( retval < 0 ) return retval;
	retval = async_read_write_test( &boards );
	if( retval < 0 ) return retval;
	retval = serial_poll_test( &boards );
	if( retval < 0 ) return retval;
	retval = parallel_poll_test( &boards );
	if( retval < 0 ) return retval;
	retval = eos_test( &boards );
	if( retval < 0 ) return retval;

	return 0;
}

