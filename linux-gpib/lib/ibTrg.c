
#include "ib_internal.h"
#include <ibP.h>

int ibtrg(int ud)
{
	uint8_t cmd = GET;
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;
	ssize_t count;

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	board = &ibBoard[conf->board];

	count = device_command(board, &cmd, 1, conf->pad, conf->sad);
	if(count != 1)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	return ibsta;
}



