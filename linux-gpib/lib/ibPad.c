
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

	if( address > 30 )
	{
		status |= ERR;
		ibsta = status;
		iberr = EARG;
		fprintf( stderr, "invalid gpib address\n" );
	}

	retval = gpibi_change_address( board, conf, address, conf->sad );
	if( retval < 0 )
	{
		status |= ERR;
		ibsta = status;
		iberr = EARG;
		fprintf( stderr, "failed to change gpib address\n" );
	}

	ibsta = status;
	return status;
}
