
#include "ib_internal.h"
#include <ibP.h>

int iblines( int ud, short *line_status )
{
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;
	int retval;

	conf = enter_library( ud, 1 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	board = interfaceBoard( conf );

	retval = ioctl( board->fileno, IBLINES, line_status );
	if( retval < 0 )
	{
		switch( errno )
		{
			default:
				setIberr( EDVR );
				break;
		}
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}
