
#include "ib_internal.h"
#include <ibP.h>

int ibpad(int ud, int v)
{
	ibConf_t *conf = ibConfigs[ud];

	ibsta &= ~ERR;

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	if ( conf->is_interface )
		return ibBoardFunc(conf->board, IBPAD, v);
	/* enable ibpad also working on devices, not only on boards */
	else
	{
		if (v >= 31)
		{
			ibsta = CMPL | ERR;
			iberr = EARG;
			ibcnt = errno;
			ibPutErrlog(-1, ibVerbCode(IBPAD));
		}else
		{
			ibConfigs[ud]->pad = v ;
		}
	}
		return ibsta;
}
