
#include "ib_internal.h"
#include <ibP.h>

int ibrsp(int ud, char *spr)
{
	ibConf_t *conf = ibConfigs[ud];

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	return ibBoardFunc( conf->board,
		(conf->flags & CN_AUTOPOLL) ? IBAPRSP : DVRSP,
		padsad(conf->pad, conf->sad), spr);
}
