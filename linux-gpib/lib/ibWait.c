
#include "ib_internal.h"
#include <ibP.h>

int ibwait( int ud, int mask )
{
	ibConf_t *conf;
	ibBoard_t *board;
	int retval;
	wait_status_ioctl_t cmd;

	conf = enter_library( ud, 0 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	board = interfaceBoard( conf );

	set_timeout( board, conf->usec_timeout);

	cmd.mask = mask;
	cmd.pad = conf->pad;
	cmd.sad = conf->sad;

	retval = ioctl( board->fileno, IBWAIT, &cmd );
	if( retval < 0 )
	{
		setIberr( EDVR );
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}
