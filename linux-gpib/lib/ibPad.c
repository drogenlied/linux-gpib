
#include "ib_internal.h"
#include <ibP.h>

int ibpad( int ud, int addr )
{
	ibConf_t *conf = ibConfigs[ ud ];
	ibBoard_t *board;
	int status = ibsta & CMPL;
	int retval;
	unsigned int address = addr;

	if( ibCheckDescriptor( ud ) < 0 )
	{
		status |= ERR;
		ibsta = status;
		iberr = EDVR;
		return status;
	}

	board = &ibBoard[ conf->board ];

	if ( conf->is_interface )
	{
		retval = ioctl( board->fileno, IBPAD, &address );
		if( retval < 0 )
		{
			status |= ERR;
			ibsta = status;
			return status;
		}
	}else
	{
		if( address > 30 )
		{
			status |= ERR;
			ibsta = status;
			iberr = EARG;
			fprintf( stderr, "invalid gpib address\n" );
		}else
		{
			conf->pad = address ;
		}
	}

	ibsta = status;
	return status;
}
