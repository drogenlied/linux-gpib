
#include "ib_internal.h"
#include <ibP.h>

int ibevent(int ud, short *event )
{
	*event = 0; // XXX
	fprintf( stderr, "libgpib: ibevent() unimplemented!\n" );
	return ibsta;
}

