
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

	if( sad > 30 )
	{
		ibsta = CMPL | ERR;
		iberr = EARG;
		ibcnt = errno;
		ibPutErrlog(-1, ibVerbCode(IBSAD));
	}

	retval = gpibi_change_address( board, conf, conf->pad, sad );
	if( retval < 0 )
	{
		status |= ERR;
		ibsta = status;
		iberr = EARG;
		fprintf( stderr, "failed to change gpib address\n" );
	}

	return ibsta;
}
