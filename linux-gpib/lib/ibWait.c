
#include "ib_internal.h"
#include <ibP.h>
#include <pthread.h>

int ibwait( int ud, int mask )
{
	ibConf_t *conf;
	ibBoard_t *board;
	int retval;
	wait_ioctl_t cmd;
	int board_wait_mask, device_wait_mask;
	int status;

	board_wait_mask = board_status_mask & ~ERR;
	device_wait_mask = device_status_mask & ~ERR;

	conf = general_enter_library( ud, 1, 0 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	board = interfaceBoard( conf );

	if( conf->is_interface == 0 &&
		board->is_system_controller == 0 )
	{
		setIberr( ECIC );
		return exit_library( ud, 1 );
	}

	cmd.usec_timeout = conf->usec_timeout;
	cmd.mask = mask;
	if( conf->is_interface == 0 )
	{
		cmd.pad = conf->pad;
		cmd.sad = conf->sad;
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
		exit_library( ud, 1 );
	}

	retval = ioctl( board->fileno, IBWAIT, &cmd );
	if( retval < 0 )
	{
		switch( errno )
		{
			case ETIMEDOUT:
				conf->timed_out = 1;
				return exit_library( ud, 0 );
				break;
			default:
				break;
		}
		setIberr( EDVR );
		return exit_library( ud, 1 );
	}

	status = exit_library( ud, 0 );

	if( conf->async.in_progress && ( status & CMPL ) )
	{
		pthread_mutex_lock( &conf->async.lock );
		conf->async.in_progress = 0;
		setIbcnt( conf->async.length );
		pthread_mutex_unlock( &conf->async.lock );
	}

	return status;
}
