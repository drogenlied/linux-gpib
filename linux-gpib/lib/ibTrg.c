
#include "ib_internal.h"
#include <ibP.h>

int ibtrg(int ud)
{
	ibConf_t *conf = ibConfigs[ud];

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	return ibBoardFunc( conf->board, DVTRG, padsad(conf->pad, conf->sad));
}

