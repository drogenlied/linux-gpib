
#include "ib_internal.h"
#include <ibP.h>

int internal_iblines( ibConf_t *conf, short *line_status )
{
	int retval;
	ibBoard_t *board;

	board = interfaceBoard( conf );

	retval = ioctl( board->fileno, IBLINES, line_status );
	if( retval < 0 )
	{
		switch( errno )
		{
			default:
				setIbcnt( errno );
				setIberr( EDVR );
				break;
		}
		return -1;
	}
	return 0;
}

int iblines( int ud, short *line_status )
{
	ibConf_t *conf;
	int retval;

	conf = enter_library( ud );
	if( conf == NULL )
		return exit_library( ud, 1 );

	retval = internal_iblines( conf, line_status );
	if( retval < 0 )
	{
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}
