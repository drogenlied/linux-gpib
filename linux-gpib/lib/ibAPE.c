
#include "ib_internal.h"
#include <ibP.h>

int ibape(int ud, int v)
{
	ibConf_t *conf = ibConfigs[ud];

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	return ibBoardFunc(conf->board, IBAPE, padsad(conf->pad, conf->sad), NULL, v);
}
