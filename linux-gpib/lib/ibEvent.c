
#include "ib_internal.h"
#include <ibP.h>

int ibevent(int ud, short *event )
{
	ibConf_t *conf;
	ibBoard_t *board;

	conf = enter_library( ud, 1 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	board = interfaceBoard( conf );

	fprintf( stderr, "libgpib: ibevent() unimplemented!\n" );
	*event = 0; // XXX

	return exit_library( ud, 0 );
}

