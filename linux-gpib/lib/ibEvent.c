
#include "ib_internal.h"
#include "ibP.h"

int ibevent(int ud, short *event )
{
	ibConf_t *conf;
	ibBoard_t *board;
	int retval;
	event_ioctl_t user_event;

	conf = general_enter_library( ud, 1, 1 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	board = interfaceBoard( conf );

	retval = ioctl( board->fileno, IBEVENT, &user_event );
	if( retval < 0 )
	{
		setIberr( EDVR );
		setIbcnt( errno );
		return exit_library(ud, 1 );
	}

	*event = user_event;

	return exit_library( ud, 0 );
}

