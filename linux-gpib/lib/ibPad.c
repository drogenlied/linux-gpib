
#include "ib_internal.h"
#include <ibP.h>

int internal_ibpad( ibConf_t *conf, unsigned int address )
{
	ibBoard_t *board;
	int retval;

	board = interfaceBoard( conf );

	if( address > 30 )
	{
		setIberr( EARG );
		fprintf( stderr, "libgpib: invalid gpib address\n" );
		return -1;
	}

	retval = gpibi_change_address( conf, address, conf->sad );
	if( retval < 0 )
	{
		fprintf( stderr, "libgpib: failed to change gpib address\n" );
		return -1;
	}

	return 0;
}

int ibpad( int ud, int addr )
{
	ibConf_t *conf;
	int retval;

	conf = enter_library( ud );
	if( conf == NULL )
		return exit_library( ud, 1 );

	retval = internal_ibpad( conf, addr );
	if( retval < 0 )
	{
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}
