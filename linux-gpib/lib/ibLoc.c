
#include "ib_internal.h"
#include <ibP.h>

int ibloc(int ud)
{
	ibConf_t *conf = ibConfigs[ud];
	uint8_t cmds[256];

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	cmds[0] = UNL;
	cmds[1] = LAD | conf->pad;
	cmds[2] = GTL;
	cmds[3] = UNL;
	return ibcmd(CONF(ud,board), cmds, 4);
} /* ibloc */
