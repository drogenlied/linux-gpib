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

int device_ppc( ibConf_t *conf, int ppc_configuration )
{
	uint8_t cmd[16];
	int i;
	int retval;

	if( interfaceBoard( conf )->is_system_controller == 0 )
	{
		setIberr( ECIC );
		return -1;
	}

	i = send_setup_string( conf, cmd );
	if( i < 0 )
	{
		setIberr( EDVR );
		return -1;
	}

	cmd[ i++ ] = PPC;
	cmd[ i++ ] = ppc_configuration;

	retval = my_ibcmd( conf, cmd, i );
	if( retval < 0 )
	{
		return -1;
	}

	return 0;
}

int board_ppc( ibConf_t *conf, int ppc_configuration )
{
	ibBoard_t *board;
	int retval;

	board = interfaceBoard( conf );

	if( conf->local_ppc == 0 )
	{
		setIberr( ECAP );
		return -1;
	}

	retval = query_ppc( board );
	if( retval < 0 ) return retval;
	conf->ppoll_config = retval;	// store old value

	retval = ioctl( board->fileno, IBPPC, &ppc_configuration );
	if( retval < 0 )
	{
		setIberr( EDVR );
		setIbcnt( errno );
		return -1;
	}

	return 0;
}

int internal_ibppc( ibConf_t *conf, int v )
{
	static const int ppc_mask = 0xe0;
	int retval;

	if( v && ( v & ppc_mask ) != PPE )
	{
		fprintf( stderr, "libgpib: illegal parallel poll configuration\n" );
		setIberr( EARG );
		return -1;
	}

	if( !v || (v & PPC_DISABLE) )
		v = PPE | PPC_DISABLE;

	if( conf->is_interface )
	{
		retval = board_ppc( conf, v );
		if( retval < 0 )
			return retval;
	}else
	{
		retval = device_ppc( conf, v );
		if( retval < 0 ) return retval;
	}

	setIberr( conf->ppoll_config );
	conf->ppoll_config = v;

	return 0;
}

int ibppc( int ud, int v )
{
	ibConf_t *conf;
	int retval;

	conf = enter_library( ud );
	if( conf == NULL )
		return exit_library( ud, 1 );

	retval = internal_ibppc( conf, v );
	if( retval < 0 ) return exit_library( ud, 1 );

	return exit_library( ud, 0 );
}
