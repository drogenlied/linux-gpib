
#include "ib_internal.h"
#include <ibP.h>

int ibcac( int ud, int synchronous )
{
	ibConf_t *conf;
	ibBoard_t *board;
	int retval;

	conf = enter_library( ud, 1 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	board = interfaceBoard( conf );

	retval = ioctl( board->fileno, IBCAC, &synchronous );
	if(retval < 0)
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
