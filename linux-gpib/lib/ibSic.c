/***************************************************************************
                             lib/ibSic.c
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

int assert_ifc( ibBoard_t *board, unsigned int usec_duration )
{
	int retval;

	retval = ioctl( board->fileno, IBSIC, &usec_duration );
	if( retval < 0 )
	{
		setIberr( EDVR );
		setIbcnt( errno );
	}
	return retval;
}

int internal_ibsic( ibConf_t *conf )
{
	ibBoard_t *board;

	if( conf->is_interface == 0 )
	{
		setIberr( EDVR );
		return -1;
	}

	board = interfaceBoard( conf );

	if( is_system_controller( board ) == 0 )
	{
		setIberr( ESAC );
		return -1;
	}

	return assert_ifc( board, 100 );
}

int ibsic(int ud)
{
	ibConf_t *conf;
	int retval;

	conf = enter_library( ud );
	if( conf == NULL )
		return exit_library( ud, 1 );

	retval = internal_ibsic( conf );
	if( retval < 0 )
	{
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}

void SendIFC( int boardID )
{
	ibsic( boardID );
}


