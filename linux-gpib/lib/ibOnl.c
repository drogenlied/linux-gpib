
#include "ib_internal.h"
#include <ibP.h>
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
	online_cmd.master = board->is_system_controller;
	online_cmd.online = online;
	retval = ioctl( board->fileno, IBONL, &online_cmd );
	if( retval < 0 )
	{
		setIberr( EDVR );
		return -1;
	}

	if( online )
	{
		if( board->is_system_controller )
		{
			// remote enable should be asserted before IFC XXX
			retval = remote_enable( board, 1 );
			if( retval < 0 ) return retval;
		}
		if( create_autopoll_thread( board ) < 0)
			return -1;
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
		conf->board_is_open = 1;
	}else
	{
		conf->board_is_open = 0;
	}

	retval = open_gpib_device( conf );
	if( retval < 0 ) return retval;

	return 0;
}

int ibonl( int ud, int onl )
{
	ibConf_t *conf;
	int retval;
	ibBoard_t *board;

	// XXX should probably lock board?
	conf = general_enter_library( ud, 1, 0 );
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


