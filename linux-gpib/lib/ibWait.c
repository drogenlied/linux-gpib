
#include "ib_internal.h"
#include <ibP.h>

//broken
int ibwait( int ud, int mask )
{
	ibConf_t *conf = ibConfigs[ ud ];
	ibBoard_t *board;
	int retval, status = ibsta & CMPL;
	unsigned int wait_mask = mask;

	if( ibCheckDescriptor( ud ) < 0 )
	{
		iberr = EDVR;
		status |= ERR;
		ibsta = status;
		return status;
	}

	board = &ibBoard[ conf->board ];

	retval = ioctl( board->fileno, IBWAIT, &wait_mask );
	if( retval < 0 )
	{
		iberr = EDVR;
		status |= ERR;
		ibsta = status;
		return status;
	}

	return wait_mask;
}
