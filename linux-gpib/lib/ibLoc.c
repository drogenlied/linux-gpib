/***************************************************************************
                          lib/ibLoc.c
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
#include "ibP.h"
#include <stdlib.h>

int ibloc(int ud)
{
	ibConf_t *conf;
	ibBoard_t *board;
	uint8_t cmd[32];
	unsigned int i;
	ssize_t count;

	conf = enter_library( ud );
	if( conf == NULL )
		return exit_library( ud, 1 );

	board = interfaceBoard( conf );

	i = send_setup_string( conf, cmd );
	if( i < 0 )
	{
		setIberr( EDVR );
		return exit_library( ud, 1 );
	}
	cmd[ i++ ] = GTL;
	count = my_ibcmd( conf, cmd, i);
	if(count != i)
	{
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );

} /* ibloc */

void EnableLocal( int boardID, const Addr4882_t addressList[] )
{
	int i;
	ibConf_t *conf;
	ibBoard_t *board;
	uint8_t *cmd;
	int count;
	int retval;
	
	conf = enter_library( boardID );
	if( conf == NULL )
	{
		exit_library( boardID, 1 );
		return;
	}
	if( addressListIsValid( addressList ) == 0 )
	{
		exit_library( boardID, 1 );
		return;
	}

	if( conf->is_interface == 0 )
	{
		setIberr( EDVR );
		exit_library( boardID, 1 );
		return;
	}

	board = interfaceBoard( conf );

	if( is_cic( board ) == 0 )
	{
		setIberr( ECIC );
		exit_library( boardID, 1 );
		return;
	}

	if( numAddresses( addressList ) == 0 )
	{
		retval = remote_enable( board, 0 );
		if( retval < 0 ) exit_library( boardID, 1 );
		else exit_library( boardID, 0 );
		return;
	}

	cmd = malloc( 16 + 2 * numAddresses( addressList ) );
	if( cmd == NULL )
	{
		setIberr( EDVR );
		setIbcnt( ENOMEM );
		exit_library( boardID, 1 );
		return;
	}

	i = create_send_setup( board, addressList, cmd );
	cmd[ i++ ] = GTL;

	//XXX detect no listeners (EBUS) error
	count = my_ibcmd( conf, cmd, i );

	free( cmd );
	cmd = NULL;

	if(count != i)
	{
		exit_library( boardID, 1 );
		return;
	}

	exit_library( boardID, 0 );
}

