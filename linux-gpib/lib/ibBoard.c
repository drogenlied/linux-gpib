
#include <stdio.h>
#include <stdlib.h>

#include "ib_internal.h"
#include <ibP.h>

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

void ibBoardDefaultValues( void )
{
	int i;
	for( i = 0; i < MAX_BOARDS; i++ )
	{
		ibBoard[ i ].pad = 0;
		ibBoard[ i ].sad = -1;
		ibBoard[ i ].base = 0;
		ibBoard[ i ].irq = 0;
		ibBoard[ i ].dma = 0;
		ibBoard[ i ].is_system_controller = 0;
		ibBoard[ i ].fileno = -1;
		strcpy( ibBoard[ i ].device, "" );
		strcpy( ibBoard[ i ].board_type, "" );
		ibBoard[ i ].autopoll_thread = NULL;
	}
}

void cleanup_autopoll( void *arg )
{
	ibBoard_t *board = arg;

	if( board->autopoll_thread )
	{
		free( board->autopoll_thread );
		board->autopoll_thread = NULL;
	}
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

	pthread_cleanup_pop( 1 );
	return NULL;
}

int create_autopoll_thread( ibBoard_t *board )
{
	int retval;

	if( board->autopoll_thread ) return 0;

	board->autopoll_thread = malloc( sizeof( pthread_t ) );
	if( board->autopoll_thread == NULL ) return -1;

	retval = pthread_create( board->autopoll_thread, NULL, run_autopoll, board );
	if( retval )
	{
		cleanup_autopoll( board );
		return -1;
	};

	pthread_detach( *board->autopoll_thread );

	return 0;
}

int destroy_autopoll_thread( ibBoard_t *board )
{
	int retval;
	
	if( board->autopoll_thread == NULL ) return 0;

	retval = pthread_cancel( *board->autopoll_thread );
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


/**********************/
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
		return -1;
	}
	board->fileno = fd;

	if( ibBdChrConfig( board ) < 0 )
	{
		setIberr( EDVR );
		setIbcnt( errno );
		fprintf( stderr, "libgpib: failed to configure board\n" );
		return -1;
	}

	if( create_autopoll_thread( board ) < 0)
	{
		ibBoardClose( board );
		return -1;
	}

	return 0;
}

/**********************/
int ibBoardClose( ibBoard_t *board )
{
	int retval;

	if( board->fileno >= 0 )
	{
		close( board->fileno );
		board->fileno = -1;
	}

	retval = destroy_autopoll_thread( board );
	if( retval < 0 )
	{
		return retval;
	}

	return 0;
}

/**********************/







