
#include "ib_internal.h"
#include <ibP.h>

int ibloc(int ud)
{
	ibConf_t *conf = ibConfigs[ud];
	ibBoard_t *board;
	uint8_t cmds[256];
	unsigned int i;
	ssize_t count;

	if(ibCheckDescriptor(ud) < 0)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	board = &ibBoard[ conf->board ];

	i = 0;
	cmds[ i++ ] = UNL;
	cmds[ i++ ] = MLA( conf->pad );
	if( conf->sad >= 0 )
		cmds[ i++ ] = MSA( conf->sad );
	cmds[ i++ ] = GTL;
	cmds[ i++ ] = UNL;
	count = my_ibcmd( conf, cmds, i);

	if(count != i)
	{
		iberr = EDVR;
		return ibsta | ERR;
	}

	return ibsta;

} /* ibloc */

