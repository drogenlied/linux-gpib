
#include "ib_internal.h"
#include <ibP.h>

int internal_ibrsv( ibConf_t *conf, int v )
{
	ibBoard_t *board;
	uint8_t status_byte = v;
	int retval;
	
	if( conf->is_interface == 0 )
	{
		setIberr( EARG );
		return -1;
	}

	board = interfaceBoard( conf );

	retval = ioctl( board->fileno, IBRSV, &status_byte );
	if( retval < 0 )
	{
		return retval;
	}

	return 0;
}

// should return old status byte in iberr on success
int ibrsv( int ud, int v )
{
	ibConf_t *conf;
	int retval;

	conf = enter_library( ud );
	if( conf == NULL )
		return exit_library( ud, 1 );

	retval = internal_ibrsv( conf, v );
	if( retval < 0 )
	{
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}

