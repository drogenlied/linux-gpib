/***************************************************************************
                          lib/ibWait.c
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
#include <pthread.h>

int my_wait( ibConf_t *conf, int mask )
{
	ibBoard_t *board;
	int retval;
	wait_ioctl_t cmd;
	int board_wait_mask, device_wait_mask;

	board_wait_mask = board_status_mask & ~ERR;
	device_wait_mask = device_status_mask & ~ERR;

	board = interfaceBoard( conf );

	if( conf->is_interface == 0 &&
		is_cic( board ) == 0 )
	{
		setIberr( ECIC );
		return -1;
	}

	cmd.usec_timeout = conf->settings.usec_timeout;
	cmd.mask = mask;
	if( conf->is_interface == 0 )
	{
		cmd.pad = conf->settings.pad;
		cmd.sad = conf->settings.sad;
		cmd.mask &= device_wait_mask;
	}else
	{
		cmd.pad = NOADDR;
		cmd.sad = NOADDR;
		cmd.mask &= board_wait_mask;
	}

	if( mask != cmd.mask )
	{
		setIberr( EARG );
		return -1;
	}

	retval = ioctl( board->fileno, IBWAIT, &cmd );
	if( retval < 0 )
	{
		switch( errno )
		{
			case ETIMEDOUT:
				conf->timed_out = 1;
				return 0;
				break;
			default:
				break;
		}
		setIberr( EDVR );
		setIbcnt( errno );
		return -1;
	}

	return 0;
}

int ibwait( int ud, int mask )
{
	ibConf_t *conf;
	int retval;
	int status;

	conf = general_enter_library( ud, 1, 0 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	retval = my_wait( conf, mask );
	if( retval < 0 )
		return exit_library( ud, 1 );

	status = general_exit_library( ud, 0, 0, mask & ( DTAS | DCAS ) );

	if( conf->async.in_progress && ( status & CMPL ) )
	{
		pthread_mutex_lock( &conf->async.lock );
		conf->async.in_progress = 0;
		setIbcnt( conf->async.length );
		pthread_mutex_unlock( &conf->async.lock );
	}

	return status;
}

void WaitSRQ( int boardID, short *result )
{
	ibConf_t *conf;
	int retval;
	int wait_mask;

	conf = enter_library( boardID );
	if( conf == NULL )
	{
		exit_library( boardID, 1 );
		return;
	}

	if( conf->is_interface == 0 )
	{
		setIberr( EDVR );
		return;
	}

	wait_mask = SRQI | TIMO;
	retval = my_wait( conf, wait_mask );
	if( retval < 0 )
	{
		exit_library( boardID, 1 );
		return;
	}
	// XXX need better query of service request state, new ioctl?
	// should play nice with autopolling
	if( ThreadIbsta() & SRQI ) *result = 1;
	else *result = 0;

	exit_library( boardID, 0 );
}
