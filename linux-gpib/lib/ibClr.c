
#include "ib_internal.h"
#include <ibP.h>

int ibclr( int ud )
{
	uint8_t cmd[ 16 ];
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;
	ssize_t count;
	unsigned int i;

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	board = &ibBoard[conf->board];

	if( conf->is_interface )
	{
		iberr = EARG;
		return ibsta | ERR;
	}

	i = 0;
	cmd[ i++ ] = UNL;
	cmd[ i++ ] = MLA( conf->pad );
	if( conf->sad >=0 )
		cmd[ i++ ] = MSA( conf->sad );
	cmd[ i++ ] = SDC;

	count = my_ibcmd( board, conf, cmd, i );
	if(count != i)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	ibsta &= ~ERR;
	return ibsta;
}


