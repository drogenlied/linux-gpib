
#include "ib_internal.h"
#include <ibP.h>

int ibdma( int ud, int v )
{
	ibConf_t *conf;
	ibBoard_t *board;

	conf = enter_library( ud, 1 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	board = interfaceBoard( conf );

	fprintf( stderr, "libgpib: ibdma() unimplemented!\n" );

	return exit_library( ud, 0 );
} /* ibdma */
