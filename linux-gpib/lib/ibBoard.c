
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
		ibBoard[ i ].autopoll_pid = 0;
	}
}

void cleanup_autopoll_processes( void )
{
	unsigned int i;
	int retval;
	for( i = 0; i < MAX_BOARDS; i++ )
	{
		if( ibBoard[ i ].autopoll_pid > 0 )
		{
			retval = kill( ibBoard[ i ].autopoll_pid, SIGTERM );
			if( retval < 0 )
			{
				fprintf( stderr, "libgpib: failed to terminate child autopoll process\n" );
			}
		}
	}
}

int fork_autopoll_process( ibBoard_t *board )
{
	pid_t process_id;
	int retval;
	static int atexit_registered = 0;

	if( board->autopoll_pid > 0 ) return 0;

	process_id = fork();
	if( process_id < 0 ) return process_id;

	board->autopoll_pid = process_id;

	if( process_id )
	{
		if( atexit_registered == 0 )
		{
			retval = atexit( cleanup_autopoll_processes );
			if( retval < 0 )
			{
				fprintf( stderr, "libgpib: failed to register atexit cleanup function\n" );
			}
			atexit_registered = 1;
		}
		return 0;
	}

	retval = ioctl( board->fileno, IBAUTOPOLL );
	exit( retval );
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
		return -1;
	}

	if( fork_autopoll_process( board ) < 0)
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

	if( board->autopoll_pid > 0 )
	{
		retval = kill( board->autopoll_pid, SIGTERM );
		if( retval < 0 )
		{
			fprintf( stderr, "libgpib: failed to terminate child autopoll process\n" );
			return retval;
		}
		board->autopoll_pid = 0;
	}

	return 0;
}

/**********************/







