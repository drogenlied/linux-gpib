
#include "ib_internal.h"
#include <ibP.h>

int ibsad(int ud, int v)
{
	ibConf_t *conf = ibConfigs[ud];

	ibsta &= ~ERR;
	
	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	if( conf->is_interface )
		return ibBoardFunc( conf->board, IBSAD, v - sad_offset);
	else
	{
		/* enable ibsad also working on devices, not only on boards */
		if ( ( v >= sad_offset && v <= sad_offset + gpib_addr_max ) ||  (v == 0x0))
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
