/***************************************************************************
                          lib/ibCmd.c
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
#include <ibP.h>
#include <sys/ioctl.h>
#include <pthread.h>

void* start_async_cmd( void *arg );

int ibcmd(int ud, void *cmd_buffer, long cnt)
{
	ibConf_t *conf;
	ibBoard_t *board;
	ssize_t count;

	conf = enter_library( ud );
	if( conf == NULL )
		return exit_library( ud, 1 );

	// check that ud is an interface board
	if( conf->is_interface == 0 )
	{
		setIberr( EARG );
		return exit_library( ud, 1 );
	}

	board = interfaceBoard( conf );

	if( board->is_system_controller == 0 )
	{
		setIberr( ECIC );
		return exit_library( ud, 1 );
	}

	count = my_ibcmd( conf, cmd_buffer, cnt);
	if(count < 0)
	{
		// report no listeners error XXX
		return exit_library( ud, 1);
	}

	if(count != cnt)
	{
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}

// XXX no timeout for asynchronous?
int ibcmda( int ud, void *cmd_buffer, long cnt )
{
	ibConf_t *conf;
	ibBoard_t *board;
	int retval;

	conf = enter_library( ud );
	if( conf == NULL )
		return exit_library( ud, 1 );

	// check that ud is an interface board
	if( conf->is_interface == 0 )
	{
		setIberr( EARG );
		return exit_library( ud, 1 );
	}

	board = interfaceBoard( conf );

	if( board->is_system_controller == 0 )
	{
		setIberr( ECIC );
		return exit_library( ud, 1 );
	}

	pthread_mutex_lock( &conf->async.lock );

	conf->async.buffer = cmd_buffer;
	conf->async.length = cnt;
	conf->async.in_progress = 1;

	retval = pthread_create( &conf->async.thread,
		NULL, start_async_cmd, conf );
	if( retval )
	{
		setIberr( EDVR );
		setIbcnt( retval );

		return exit_library( ud, 1 );
	}
	pthread_detach( conf->async.thread );

	return general_exit_library( ud, 0, 1 );
}

void* start_async_cmd( void *arg )
{
	long count;
	ibConf_t *conf;
	int retval;

	conf = arg;

	// XXX my_ibcmd fiddles with iberr
	count = my_ibcmd( conf, conf->async.buffer, conf->async.length );
	if(count < 0)
	{
		conf->async.length = 0;
	}else
	{
		conf->async.length = count;
	}

	conf->has_lock = 0;
	retval = unlock_board_mutex( interfaceBoard( conf ) );
	if( retval < 0 )
	{
		conf->has_lock = 1;
		conf->async.length = 0;
		conf->async.error = EDVR;
	}

	pthread_mutex_unlock( &conf->async.lock );

	return NULL;
}

ssize_t my_ibcmd( ibConf_t *conf, uint8_t *buffer, size_t count)
{
	read_write_ioctl_t cmd;
	int retval;
	ibBoard_t *board;

	board = interfaceBoard( conf );

	// check that interface board is master
	if( board->is_system_controller == 0 )
	{
		setIberr( ECIC );
		return -1;
	}

	cmd.buffer = buffer;
	cmd.count = count;

	set_timeout( board, conf->usec_timeout);

	retval = ioctl( board->fileno, IBCMD, &cmd );
	if( retval < 0 )
	{
		switch( errno )
		{
			case ETIMEDOUT:
				setIberr( EABO );
				conf->timed_out = 1;
				break;
			default:
				setIberr( EDVR );
				setIbcnt( errno );
				break;
		}
		return -1;
	}

	return cmd.count;
}

unsigned int create_send_setup( const ibBoard_t *board,
	Addr4882_t addressList[], uint8_t *cmdString )
{
	unsigned int i, j;

	if( addressList == NULL ) return 0;

	if( addressListIsValid( addressList ) == 0 )
	{
		fprintf(stderr, "libgpib: bug! bad address list\n");
		return 0;
	}

	i = 0;
	cmdString[ i++ ] = UNL;
	for( j = 0; j < numAddresses( addressList ); j++ )
	{
		unsigned int pad;
		int sad;

		pad = extractPAD( addressList[ j ] );
		sad = extractSAD( addressList[ j ] );
		cmdString[ i++ ] = MLA( pad );
		if( sad >= 0)
			cmdString[ i++ ] = MSA( sad );
	}
	/* controller's talk address */
	cmdString[ i++ ] = MTA( board->pad );
	if( board->sad >= 0 )
		cmdString[ i++ ] = MSA( board->sad );

	return i;
}

int send_setup_string( const ibConf_t *conf,
	uint8_t *cmdString )
{
	ibBoard_t *board;
	Addr4882_t addressList[ 2 ];

	board = interfaceBoard( conf );

	addressList[ 0 ] = packAddress( conf->pad, conf->sad );
	addressList[ 1 ] = NOADDR;

	return create_send_setup( board, addressList, cmdString );
}

int send_setup( ibConf_t *conf )
{
	uint8_t cmdString[8];
	int retval;

	retval = send_setup_string( conf, cmdString );
	if( retval < 0 ) return retval;

	if( my_ibcmd( conf, cmdString, retval ) < 0 )
		return -1;

	return 0;
}
