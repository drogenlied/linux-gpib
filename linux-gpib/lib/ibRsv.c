
#include "ib_internal.h"
#include <ibP.h>

// should return old status byte in iberr on success
int ibrsv( int ud, int v )
{
	ibConf_t *conf;
	ibBoard_t *board;
	int retval;
	uint8_t status_byte = v;

	conf = enter_library( ud, 1 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	board = interfaceBoard( conf );

	retval = ioctl( board->fileno, IBRSV, &status_byte );
	if( retval < 0 )
	{
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}

