
#include "ib_internal.h"
#include <ibP.h>

int ibloc(int ud)
{
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;
	uint8_t cmds[256];

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	board = &ibBoard[ conf->board ];

	cmds[0] = UNL;
	cmds[1] = LAD | conf->pad;
// XXX fix for sad
	cmds[2] = GTL;
	cmds[3] = UNL;
	return __ibcmd(board, conf, cmds, 4);
} /* ibloc */

