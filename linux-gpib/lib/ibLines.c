
#include "ib_internal.h"
#include <ibP.h>

int iblines( int ud, short *line_status )
{
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;
	int retval;
	int status = CMPL;

	if( ibCheckDescriptor( ud ) < 0 )
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	board = &ibBoard[ conf->board ];

	retval = ioctl( board->fileno, IBLINES, line_status );
	if( retval < 0 )
	{
		switch( errno )
		{
			default:
				iberr = EDVR;
				break;
		}
		status |= ERR;
	}

	ibsta = status;
	return status;
}
