/***************************************************************************
                          lib/ibOnl.c
                             -------------------

    copyright            : (C) 2001,2002,2003 by Frank Mori Hess
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

int board_online( ibBoard_t *board, int online )
{
	online_ioctl_t online_cmd;
	int retval;

	if( online )
	{
		if( ibBoardOpen( board ) < 0 )
			return -1;
		if( ibBdChrConfig( board ) < 0 )
		{
			setIberr( EDVR );
			setIbcnt( errno );
			fprintf( stderr, "libgpib: failed to configure board\n" );
			ibBoardClose( board );
			return -1;
		}
	}else
	{
		retval = destroy_autopoll_thread( board );
		if( retval < 0 )
			return retval;
	}
	online_cmd.online = online;
	retval = ioctl( board->fileno, IBONL, &online_cmd );
	if( retval < 0 )
	{
		fprintf( stderr, "libgpib: IBONL ioctl failed\n" );
		setIberr( EDVR );
		setIbcnt( errno );
		return -1;
	}

	if( online )
	{
		retval = lock_board_mutex( board );
		if( retval < 0 ) return retval;

		retval = request_system_control( board, board->is_system_controller );
		if( retval < 0 )
		{
			unlock_board_mutex( board );
			return retval;
		}
		if( board->is_system_controller )
		{
			retval = remote_enable( board, 1 );
			if( retval < 0 )
			{
				unlock_board_mutex( board );
				return retval;
			}
			retval = assert_ifc( board, 100 );
			if( retval < 0 )
			{
				unlock_board_mutex( board );
				return retval;
			}
		}
		if( create_autopoll_thread( board ) < 0)
		{
			unlock_board_mutex( board );
			return -1;
		}
		retval = unlock_board_mutex( board );
		if( retval < 0 ) return retval;
	}else ibBoardClose( board );

	return 0;
}

int conf_online( ibConf_t *conf, int online )
{
	ibBoard_t *board;
	int retval;

	if( ( online && conf->board_is_open ) ||
		( online == 0 && conf->board_is_open == 0 ) )
		return 0;

	board = interfaceBoard( conf );

	retval = board_online( board, online );
	if( retval < 0 ) return retval;

	if( online )
	{
		retval = conf_lock_board( conf );
		if( retval < 0 ) return retval;

		conf->board_is_open = 1;
		retval = open_gpib_device( conf );
		if( retval < 0 ) return retval;

		retval = conf_unlock_board( conf );
		if( retval < 0 ) return retval;
	}else
	{
		conf->board_is_open = 0;
	}

	return 0;
}

int ibonl( int ud, int onl )
{
	ibConf_t *conf;
	int retval;
	ibBoard_t *board;

	conf = enter_library( ud );
	if( conf == NULL )
		return exit_library( ud, 1 );

	// XXX supposed to reset board/device if onl is nonzero
	if( onl ) return exit_library( ud, 0 );

	board = interfaceBoard( conf );

	retval = close_gpib_device( conf );
	if( retval < 0 )
	{
		fprintf( stderr, "libgpib: failed to mark device as closed!\n" );
		setIberr( EDVR );
		setIbcnt( errno );
		return exit_library( ud, 1 );
	}

	retval = conf_online( conf, onl );
	if( retval < 0 )
	{
		return exit_library( ud, 1 );
	}

	if( ud >= MAX_BOARDS )
	{
		// need to take more care to clean up before freeing XXX
		free( ibConfigs[ ud ] );
		ibConfigs[ ud ] = NULL;
	}
	setIbsta( 0 );
	return 0;
}


