
#include "ib_internal.h"
#include <ibP.h>
#include <sys/ioctl.h>

unsigned int timeout_to_usec( enum gpib_timeout timeout )
{
	switch ( timeout )
	{
		default:
		case TNONE:
			return 0;
			break;
		case T10us:
			return 10;
			break;
		case T30us:
			return 30;
			break;
		case T100us:
			return 100;
			break;
		case T300us:
			return 300;
			break;
		case T1ms:
			return 1000;
			break;
		case T3ms:
			return 3000;
			break;
		case T10ms:
			return 10000;
			break;
		case T30ms:
			return 30000;
			break;
		case T100ms:
			return 100000;
			break;
		case T300ms:
			return 300000;
			break;
		case T1s:
			return 1000000;
			break;
		case T3s:
			return 3000000;
			break;
		case T10s:
			return 10000000;
			break;
		case T30s:
			return 30000000;
			break;
		case T100s:
			return 100000000;
			break;
		case T300s:
			return 300000000;
			break;
		case T1000s:
			return 1000000000;
			break;
	}
	return 0;
}

int ibtmo(int ud,int v)
{
	ibConf_t *conf;

	if( (v < TNONE) || (v > T1000s) )
	{
		setIberr( EARG );
		return exit_library( ud, 1 );
	}

	conf = enter_library( ud, 1 );
	if( conf == NULL )
		return exit_library( ud, 1 );

	conf->usec_timeout = timeout_to_usec( v );

	return exit_library( ud, 0 );
}

int set_timeout( const ibBoard_t *board, unsigned int usec_timeout)
{
       return ioctl( board->fileno, IBTMO, &usec_timeout);
}


