
#include "ib_internal.h"
#include <ibP.h>

int ibcac( int ud, int synchronous )
{
	ibConf_t *conf;
	ibBoard_t *board;
	int retval;

	conf = enter_library( ud );
	if( conf == NULL )
		return exit_library( ud, 1 );

	if( conf->is_interface == 0 )
	{
		setIberr( EARG );
		return exit_library( ud, 1 );
	}

	board = interfaceBoard( conf );

	if( board->is_system_controller == 0 )
	{
		setIberr( ECIC );
		return exit_library( ud, 1 );
	}

	retval = ioctl( board->fileno, IBCAC, &synchronous );
	// if synchronous failed, fall back to asynchronous
	if( retval < 0 && synchronous  )
	{
		synchronous = 0;
		retval = ioctl( board->fileno, IBCAC, &synchronous );
	}
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
