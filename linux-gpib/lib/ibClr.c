
#include "ib_internal.h"
#include <ibP.h>

int ibclr(int ud)
{
	uint8_t cmd = SDC;
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;
	ssize_t count;

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	board = &ibBoard[conf->board];

	count = __ibcmd( board, conf, &cmd, 1 );
	if(count != 1)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	ibsta &= ~ERR;
	return ibsta;
}


