
#include "ib_internal.h"
#include <ibP.h>

int ibsad(int ud, int v)
{
	ibConf_t *conf = ibConfigs[ud];
	const sad_offset = 0x60;
	const sad_max = 30;

	ibsta &= ~ERR;
	
	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	if( conf->flags & CN_ISCNTL)
		return ibBoardFunc( conf->board, IBSAD, v - sad_offset);
	else
	{
		/* enable ibsad also working on devices, not only on boards */
		if ( ( v >= sad_offset && v <= sad_offset + sad_max ) ||  (v == 0x0))
		{
			ibBoard[ud].sad = v - sad_offset;
		}
		else
		{
			ibsta = CMPL | ERR;
			iberr = EARG;
			ibcnt = errno;
			ibPutErrlog(-1, ibVerbCode(IBSAD));
		}
   }
	return ibsta;
}
