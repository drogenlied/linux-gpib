
#include "ib_internal.h"
#include <ibP.h>

int ibrsp(int ud, char *spr)
{
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;
	serial_poll_ioctl_t poll_cmd;
	int retval;
	int status = CMPL;

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	board = &ibBoard[ conf->board ];

	poll_cmd.pad = conf->pad;
	poll_cmd.sad = conf->sad;

	set_timeout( board, conf->usec_timeout );

	retval = ioctl( board->fileno, IBRSP, &poll_cmd );
	if(retval < 0)
	{
		switch( errno )
		{
			case ETIMEDOUT:
				status |= TIMO;
				iberr = EABO;
				break;
			default:
				iberr = EDVR;
				break;
		}
		status |= ERR;
		ibsta = status;
		return status;
	}

	*spr = poll_cmd.status_byte;

	ibsta = status;
	return status;
}
