
#include "ib_internal.h"
#include <ibP.h>

int ibrsp(int ud, char *spr)
{
	ibConf_t *conf;
	ibBoard_t *board;
	serial_poll_ioctl_t poll_cmd;
	int retval;

	conf = enter_library( ud, 1 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	board = interfaceBoard( conf );

	poll_cmd.pad = conf->pad;
	poll_cmd.sad = conf->sad;

	set_timeout( board, conf->spoll_usec_timeout );

	retval = ioctl( board->fileno, IBRSP, &poll_cmd );
	if(retval < 0)
	{
		switch( errno )
		{
			case ETIMEDOUT:
				conf->timed_out = 1;
				setIberr( EABO );
				break;
			default:
				setIberr( EDVR );
				break;
		}
		return exit_library( ud, 1 );
	}

	*spr = poll_cmd.status_byte;

	return exit_library( ud, 0 );
}
