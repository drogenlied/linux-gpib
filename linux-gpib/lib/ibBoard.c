/***************************************************************************
                          lib/ibBoard.c
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

#include <stdio.h>
#include <stdlib.h>

#include "ib_internal.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>

ibBoard_t ibBoard[ MAX_BOARDS ];
void cleanup_autopoll( void *arg );

void atfork_autopoll_prepare( void )
{
	int i;

	for( i = 0; i < MAX_BOARDS; i++ )
		pthread_mutex_lock( &ibBoard[ i ].autopoll_lock );
}

void atfork_autopoll_parent( void )
{
	int i;

	for( i = 0; i < MAX_BOARDS; i++ )
		pthread_mutex_unlock( &ibBoard[ i ].autopoll_lock );
}

void atfork_autopoll_child( void )
{
	int i, start_autopoll;

	for( i = 0; i < MAX_BOARDS; i++ )
	{
		pthread_mutex_init( &ibBoard[ i ].autopoll_lock, NULL );
		if( ibBoard[ i ].autopoll_thread )
			start_autopoll = 1;
		else
			start_autopoll = 0;
		cleanup_autopoll( &ibBoard[ i ] );
		if( start_autopoll )
			create_autopoll_thread( &ibBoard[ i ] );
	}
}

void init_ibboard( ibBoard_t *board )
{
	board->base = 0;
	board->irq = 0;
	board->dma = 0;
	board->is_system_controller = 0;
	board->fileno = -1;
	strcpy( board->device, "" );
	strcpy( board->board_type, "" );
	board->autopoll_thread = NULL;
	pthread_mutex_init( &board->autopoll_lock, NULL );
	board->pci_bus = -1;
	board->pci_slot = -1;
	board->use_event_queue = 0;
}

int initIbBoardAtFork( void )
{
	if( pthread_atfork( atfork_autopoll_prepare, atfork_autopoll_parent,
		atfork_autopoll_child ) )
	{
		perror( "pthread_atfork()" );
		return -1;
	}

	return 0;
}

void cleanup_autopoll( void *arg )
{
	ibBoard_t *board = arg;

	pthread_mutex_lock( &board->autopoll_lock );
	if( board->autopoll_thread )
	{
		free( board->autopoll_thread );
		board->autopoll_thread = NULL;
	}
	pthread_mutex_unlock( &board->autopoll_lock );
}

void * run_autopoll( void *arg )
{
	ibBoard_t *board = arg;
	int retval;

	pthread_cleanup_push( cleanup_autopoll, (void*) board );

	pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, NULL );
	pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, NULL );

	retval = ioctl( board->fileno, IBAUTOPOLL );
	if( retval )
	{
		fprintf( stderr, "libgpib: autopoll ioctl returned error!\n" );
	}

	pthread_setcanceltype( PTHREAD_CANCEL_DEFERRED, NULL );

	pthread_cleanup_pop( 1 );

	return NULL;
}

int create_autopoll_thread( ibBoard_t *board )
{
	int retval;

	pthread_mutex_lock( &board->autopoll_lock );

	if( board->autopoll_thread )
	{
		pthread_mutex_unlock( &board->autopoll_lock );
		return 0;
	}

	board->autopoll_thread = malloc( sizeof( pthread_t ) );
	if( board->autopoll_thread == NULL )
	{
		pthread_mutex_unlock( &board->autopoll_lock );
		return -1;
	}

	retval = pthread_create( board->autopoll_thread, NULL, run_autopoll, board );
	if( retval )
	{
		fprintf( stderr, "libgpib: failed to create autopoll thread, retval=%i\n", retval );
		pthread_mutex_unlock( &board->autopoll_lock );
		cleanup_autopoll( board );
		return -1;
	};

	pthread_detach( *board->autopoll_thread );

	pthread_mutex_unlock( &board->autopoll_lock );

	return 0;
}

int destroy_autopoll_thread( ibBoard_t *board )
{
	int retval;

	pthread_mutex_lock( &board->autopoll_lock );

	if( board->autopoll_thread == NULL )
	{
		pthread_mutex_unlock( &board->autopoll_lock );
		return 0;
	}

	retval = pthread_cancel( *board->autopoll_thread );

	pthread_mutex_unlock( &board->autopoll_lock );

	if( retval )
	{
		fprintf( stderr, "libgpib: failed to terminate autopoll thread\n" );
		return -1;
	}

	return 0;
}

int configure_autopoll( ibConf_t *conf, int enable )
{
	if( enable )
		return create_autopoll_thread( interfaceBoard( conf ) );
	else
		return destroy_autopoll_thread( interfaceBoard( conf ) );
}

int ibBoardOpen( ibBoard_t *board )
{
	int fd;
	int flags = 0;

	if( board->fileno >= 0 ) return 0;

	if( ( fd = open( board->device, O_RDWR | flags ) ) < 0 )
	{
		setIberr( EDVR );
		setIbcnt( errno );
		fprintf( stderr, "libgpib: ibBoardOpen failed to open device file\n" );
		perror( "libgpib" );
		return -1;
	}
	board->fileno = fd;
	board->open_count++;

	return 0;
}

int ibBoardClose( ibBoard_t *board )
{

	if( board->open_count == 0 )
	{
		fprintf( stderr, "libgpib: bug! board->open_count is zero on close\n");
		return -1;
	}

	board->open_count--;
	if( board->open_count > 0 )
		return 0;

	if( board->fileno >= 0 )
	{
		close( board->fileno );
		board->fileno = -1;
	}

	return 0;
}

int InternalResetSys( ibConf_t *conf, const Addr4882_t addressList[] )
{
	ibBoard_t *board;
	int retval;

	board = interfaceBoard( conf );

	if( addressListIsValid( addressList ) == 0 )
	{
		setIberr( EARG );
		return -1;
	}

	if( conf->is_interface == 0 )
	{
		setIberr( EDVR );
		return -1;
	}

	if( is_system_controller( board ) == 0 )
	{
		setIberr( ESAC );
		return -1;
	}

	if( is_cic( board ) == 0 )
	{
		setIberr( ECIC );
		return -1;
	}

	retval = remote_enable( board, 1 );
	if( retval < 0 ) return retval;

	retval = internal_ibsic( conf );
	if( retval < 0 ) return retval;

	retval = InternalDevClearList( conf, NULL );
	if( retval < 0 ) return retval;

	retval = InternalSendList( conf, addressList, "*RST", 4, NLend );
	if( retval < 0 ) return retval;

	return 0;
}

void ResetSys( int boardID, const Addr4882_t addressList[] )
{
	ibConf_t *conf;
	int retval;

	conf = enter_library( boardID );
	if( conf == NULL )
	{
		exit_library( boardID, 1 );
		return;
	}

	retval = InternalResetSys( conf, addressList );
	if( retval < 0 )
	{
		exit_library( boardID, 1 );
		return;
	}

	exit_library( boardID, 0 );

}







