
#include "ib_internal.h"
#include <ibP.h>

// incomplete XXX need to implement acceptor handshake stuff in drivers
int ibgts(int ud, int v)
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

	retval = ioctl( board->fileno, IBGTS, &v );
	if( retval < 0 )
	{
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}

