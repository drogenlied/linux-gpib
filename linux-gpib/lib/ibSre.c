
#include "ib_internal.h"
#include <ibP.h>

int remote_enable( const ibBoard_t *board, int enable )
{
	int retval;

	if( board->is_system_controller == 0 )
	{
		// XXX we don't distinguish ECIC and ESAC?
		setIberr( ESAC );
		return -1;
	}

	retval = ioctl( board->fileno, IBSRE, &enable );
	if( retval < 0 )
	{
		// XXX other error types?
		setIberr( EDVR );
		setIbcnt( errno );
		return retval;
	}

	return 0;
}

int internal_ibsre( ibConf_t *conf, int v )
{
	ibBoard_t *board;
	int retval;

	board = interfaceBoard( conf );

	if( conf->is_interface == 0 )
	{
		setIberr( EARG );
		return -1;
	}

	retval = remote_enable( board, v );
	if( retval < 0 )
		return retval;

	return 0;
}

int ibsre(int ud, int v)
{
	ibConf_t *conf;
	int retval;

	conf = enter_library( ud );
	if( conf == NULL )
		return exit_library( ud, 1 );

	retval = internal_ibsre( conf, v );
	if( retval < 0 )
	{
		fprintf( stderr, "libgpib: ibsre error\n");
		return exit_library( ud, 1 );
	}

	return exit_library( ud, 0 );
}

