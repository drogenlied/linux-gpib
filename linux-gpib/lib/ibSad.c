
#include "ib_internal.h"
#include <ibP.h>

int ibsad( int ud, int v )
{
	ibConf_t *conf;
	ibBoard_t *board;
	int retval;
	int sad = v - sad_offset;

	conf = enter_library( ud, 1 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	board = interfaceBoard( conf );

	if( sad > 30 )
	{
		setIberr( EARG );
		return exit_library( ud, 1 );
	}

	retval = gpibi_change_address( conf, conf->pad, sad );
	if( retval < 0 )
	{
		fprintf( stderr, "failed to change gpib address\n" );
		setIberr( EARG );
		setIbcnt( errno );
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}
