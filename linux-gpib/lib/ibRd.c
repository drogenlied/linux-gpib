
#include <stdio.h>
#include "ib_internal.h"
#include <ibP.h>

int ibrd(int ud, void *rd, unsigned long cnt)
{
	ibConf_t *conf = ibConfigs[ud];

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	// set eos mode
	iblcleos(ud);
	// set timeout XXX need to init conf with board default when not set
	ibBoardFunc(conf->board, IBTMO, conf->tmo);

	return ibBoardFunc(conf->board,
		((conf->flags & CN_ISCNTL) ? IBRD : DVRD),
		padsad(conf->pad, conf->sad), rd, cnt);
}




