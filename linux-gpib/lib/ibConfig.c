
#include "ib_internal.h"
#include <ibP.h>

int ibconfig( int ud, int option, int value )
{
	ibConf_t *conf;
	ibBoard_t *board;

	conf = enter_library( ud, 1 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	board = interfaceBoard( conf );

	fprintf( stderr, "libgpib: ibconfig() unimplemented!\n" );

	return exit_library( ud, 0 );
} /* ibconfig */

