
#include "ib_internal.h"
#include <ibP.h>

int ibsre(int ud, int v)
{
	ibConf_t *conf;
	ibBoard_t *board;
	int retval;

	conf = enter_library( ud, 1 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	board = interfaceBoard( conf );

	retval = ioctl( board->fileno, IBSRE, &v );
	if( retval < 0 )
	{
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}

