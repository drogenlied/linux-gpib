
#include "ib_internal.h"
#include <ibP.h>

int ibclr(int ud)
{
	ibConf_t *conf = ibConfigs[ud];

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	return ibBoardFunc( conf->board, DVCLR, padsad(conf->pad, conf->sad));
}
