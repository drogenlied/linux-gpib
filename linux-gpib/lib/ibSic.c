
#include "ib_internal.h"
#include <ibP.h>

int ibsic(int ud)
{
	ibConf_t *conf;
	ibBoard_t *board;
	int retval;

	conf = enter_library( ud, 1 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	board = interfaceBoard( conf );

	retval = ioctl( board->fileno, IBSIC );
	if( retval < 0 )
	{
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}

