/***************************************************************************
                          lib/ibClr.c
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

int ibclr( int ud )
{
	uint8_t cmd[ 16 ];
	ibConf_t *conf;
	ibBoard_t *board;
	ssize_t count;
	int i;

	conf = enter_library( ud, 1 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	board = interfaceBoard( conf );

	if( conf->is_interface )
	{
		setIberr( EARG );
		return exit_library( ud, 1 );
	}

	i = send_setup_string( conf, cmd );
	if( i < 0 )
	{
		setIberr( EDVR );
		return exit_library( ud, 1 );
	}
	cmd[ i++ ] = SDC;

	count = my_ibcmd( conf, cmd, i );
	if(count != i)
	{
		setIberr( EDVR );
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}


