
#include "ib_internal.h"
#include <ibP.h>

int ibcac( int ud, int synchronous )
{
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;
	int retval;
	int status = CMPL;

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	board = &ibBoard[ conf->board ];

	retval = ioctl( board->fileno, IBCAC, &synchronous );
	if(retval < 0)
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
