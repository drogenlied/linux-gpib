
#include "ib_internal.h"
#include <ibP.h>

int ibrpp(int ud, char *ppr)
{
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;

	if( ibCheckDescriptor( ud ) < 0 )
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	board = &ibBoard[ conf->board ];

	__ibtmo( board, conf->timeout );

	return ibBoardFunc(CONF(ud,board),IBRPP,0 ,ppr);
}
