
#include "ib_internal.h"
#include <ibP.h>

int assert_ifc( ibBoard_t *board, unsigned int usec_duration )
{
	int retval;

	retval = ioctl( board->fileno, IBSIC, &usec_duration );
	if( retval < 0 )
	{
		setIberr( EDVR );
		setIbcnt( errno );
	}
	return retval;
}

int internal_ibsic( ibConf_t *conf )
{
	ibBoard_t *board;

	board = interfaceBoard( conf );

	return assert_ifc( board, 100 );
}

int ibsic(int ud)
{
	ibConf_t *conf;
	int retval;

	conf = enter_library( ud );
	if( conf == NULL )
		return exit_library( ud, 1 );

	retval = internal_ibsic( conf );
	if( retval < 0 )
	{
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}

