
#include "ib_internal.h"
#include <ibP.h>

int internal_ibgts( ibConf_t *conf, int shadow_handshake )
{
	ibBoard_t *board;
	int retval;

	board = interfaceBoard( conf );

	if( is_system_controller( board ) == 0 )
	{
		setIberr( ECIC );
		return -1;
	}

	retval = ioctl( board->fileno, IBGTS, &shadow_handshake );
	if( retval < 0 )
	{
		setIberr( EDVR );
		setIbcnt( errno );
		return -1;
	}

	return 0;
}

// incomplete XXX need to implement acceptor handshake stuff in drivers
int ibgts( int ud, int v )
{
	ibConf_t *conf;
	int retval;

	conf = enter_library( ud );
	if( conf == NULL )
		return exit_library( ud, 1 );

	if( conf->is_interface == 0 )
	{
		setIberr( EARG );
		return exit_library( ud, 1 );
	}

	retval = internal_ibgts( conf, v );
	if( retval < 0 )
	{
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}

