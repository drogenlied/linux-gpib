
#include "ib_internal.h"
#include <ibP.h>

int ibtrg(int ud)
{
	uint8_t cmd[ 16 ];
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;
	ssize_t count;
	int i;

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

	i = send_setup_string( conf, cmd );
	if( i < 0 )
	{
		iberr = EDVR;
		return ibsta | ERR;
	}
	cmd[ i++ ] = GET;

	count = my_ibcmd( conf, cmd, i );
	if(count != i)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	return ibsta;
}



