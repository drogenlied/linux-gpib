
#include "ib_internal.h"
#include <ibP.h>

int ibwait( int ud, int mask )
{
	ibConf_t *conf = ibConfigs[ ud ];
	ibBoard_t *board;
	int retval, status = ibsta & CMPL;
	wait_ioctl_t cmd;

	if( ibCheckDescriptor( ud ) < 0 )
	{
		iberr = EDVR;
		status |= ERR;
		ibsta = status;
		return status;
	}

	board = &ibBoard[ conf->board ];

	set_timeout( board, conf->usec_timeout);

	cmd.mask = mask;
	cmd.pad = conf->pad;
	cmd.sad = conf->sad;

	retval = ioctl( board->fileno, IBWAIT, &cmd );
	if( retval < 0 )
	{
		iberr = EDVR;
		status |= ERR;
		ibsta = status;
		return status;
	}

	ibsta = cmd.mask;
	return cmd.mask;
}
