
#include "ib_internal.h"
#include <ibP.h>

int ibsic(int ud)
{
	ibConf_t *conf = ibConfigs[ ud ];
	ibBoard_t *board;
	int retval;
	int status = ibsta & CMPL;

	if( ibCheckDescriptor( ud ) < 0 )
	{
		status |= ERR;
		ibsta = status;
		iberr = EDVR;
		return status;
	}

	board = &ibBoard[ conf->board ];

	retval = ioctl( board->fileno, IBSIC );
	if( retval < 0 )
	{
		status |= ERR;
	}

	ibsta = status;
	return status;
}

