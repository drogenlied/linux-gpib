
#include "ib_internal.h"
#include <ibP.h>

int internal_ibsad( ibConf_t *conf, int address )
{
	ibBoard_t *board;
	int sad = address - sad_offset;
	int retval;

	board = interfaceBoard( conf );

	if( sad > 30 )
	{
		setIberr( EARG );
		return -1;
	}

	retval = gpibi_change_address( conf, conf->pad, sad );
	if( retval < 0 )
	{
		fprintf( stderr, "libgpib: failed to change gpib address\n" );
		return -1;
	}
	return 0;
}

int ibsad( int ud, int v )
{
	ibConf_t *conf;
	int retval;

	conf = enter_library( ud );
	if( conf == NULL )
		return exit_library( ud, 1 );

	retval = internal_ibsad( conf, v );
	if( retval < 0 )
	{
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}
