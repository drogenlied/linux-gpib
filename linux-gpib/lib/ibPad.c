
#include "ib_internal.h"
#include <ibP.h>

int ibpad( int ud, int addr )
{
	ibConf_t *conf;
	ibBoard_t *board;
	int retval;
	unsigned int address = addr;

	conf = enter_library( ud, 1 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	board = interfaceBoard( conf );

	if( address > 30 )
	{
		setIberr( EARG );
		fprintf( stderr, "invalid gpib address\n" );
		return exit_library( ud, 1 );
	}

	retval = gpibi_change_address( board, conf, address, conf->sad );
	if( retval < 0 )
	{
		setIberr( EARG );
		fprintf( stderr, "failed to change gpib address\n" );
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}
