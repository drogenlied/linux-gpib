
#include "ib_internal.h"
#include <ibP.h>

int ibrsv(int ud, int v)
{
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;
	int retval;
	int status = ibsta & CMPL;
	uint8_t status_byte;

	if( ibCheckDescriptor( ud ) < 0 )
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	board = &ibBoard[ conf->board ];

	retval = ioctl( board->fileno, IBRSV, &status_byte );
	if( retval < 0 )
	{
		status |= ERR;
		ibsta = status;
	}

	return status;
}

