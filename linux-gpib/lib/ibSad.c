
#include "ib_internal.h"
#include <ibP.h>

int ibsad( int ud, int v )
{
	ibConf_t *conf = ibConfigs[ ud ];
	ibBoard_t *board;
	int status = ibsta & CMPL;
	int retval;
	int sad = v - sad_offset;

	if( ibCheckDescriptor( ud ) < 0 )
	{
		status |= ERR;
		ibsta = status;
		iberr = EDVR;
		return status;
	}

	board = &ibBoard[ conf->board ];

	if( conf->is_interface )
	{
		retval = ioctl( board->fileno, IBSAD, &sad );
		if( retval < 0 )
		{
			status |= ERR;
			ibsta = status;
			iberr = EDVR;
			return status;
		}
	}else
	{
		if ( sad_offset <= 30 )
		{
			conf->sad = sad;
			if( conf->sad < 0 ) conf->sad = -1;
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
