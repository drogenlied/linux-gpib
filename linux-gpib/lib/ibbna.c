/***************************************************************************
                          lib/ibbna.c
                             -------------------

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

int ibbna( int ud, char *board_name )
{
	ibConf_t *conf, *board_conf;
	ibBoard_t *board;
	int retval;
	int find_index;
	int old_board_index;

	conf = enter_library( ud, 1 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	board = interfaceBoard( conf );

	if( conf->is_interface )
	{
		setIberr( EARG );
		return exit_library( ud, 1 );
	}

	retval = close_gpib_device( conf );
	if( retval < 0 )
	{
		setIberr( EDVR );
		return exit_library( ud, 1 );
	}

	if( ( find_index = ibFindDevIndex( board_name ) ) < 0 )
	{
		setIberr( EARG );
		return exit_library( ud, 1 );
	}

	board_conf = &ibFindConfigs[ find_index ];
	if( board_conf->is_interface == 0 )
	{
		setIberr( EARG );
		return exit_library( ud, 1 );
	}
	if( interfaceBoard( board_conf )->is_system_controller == 0 )
	{
		setIberr( ECIC );
		return exit_library( ud, 1 );
	}

	old_board_index = conf->board;
	conf->board = board_conf->board;

	if( ibBoardOpen( interfaceBoard( conf ) ) < 0 )
	{
		setIberr( EDVR );
		return exit_library( ud, 1 );
	}

	retval = open_gpib_device( conf );
	if( retval < 0 )
	{
		setIberr( EDVR );
		return exit_library( ud, 1 );
	}

	setIberr( old_board_index );
	
	return exit_library( ud, 0 );
}
