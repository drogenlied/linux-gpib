/***************************************************************************
                              lib/ibppc.c
                             -------------------

    begin                : Oct 2002
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
#include "ibP.h"

int ibppc( int ud, int v )
{
	ibConf_t *conf;
	uint8_t cmd[16];
	static const int ppc_mask = 0xe0;
	static const int ppc_code = 0x60;
	int i;
	int retval;

	if( v && ( v & ppc_mask ) != ppc_code )
	{
		fprintf( stderr, "libgpib: illegal parallel poll configuration\n" );
		setIberr( EARG );
		return exit_library( ud, 1 );
	}

	conf = enter_library( ud, 1 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	if( interfaceBoard( conf )->is_system_controller == 0 )
	{
		setIberr( ECIC );
		return exit_library( ud, 1 );
	}

	i = send_setup_string( conf, cmd );
	if( i < 0 )
	{
		setIberr( EDVR );
		return exit_library( ud, 1 );
	}

	if( v )
	{
		cmd[ i++ ] = PPC;
		cmd[ i++ ] = v;
	}else
		cmd[ i++ ] = PPU;
		
	retval = my_ibcmd( conf, cmd, i );
	if( retval < 0 )
	{
		setIberr( EDVR );
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}