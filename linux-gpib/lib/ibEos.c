
#include <ib.h>
#include <ibP.h>

int ibeos(int ud, int v)
{
	ibConf_t *conf = ibConfigs[ud];

	ibsta &= ~ERR;

	if(ibCheckDescriptor(ud) < 0)
	{
		ibsta |= ERR;
		return ibsta;
	}

	conf->eos = v & 0xff;
	conf->eosflags = (v >> 8) & 0xff;

	return ibsta;
}

/*
 *  iblcleos()
 *  sets the eos modes of the unit description (local) or
 *  board description (if none)
 *
 */

int iblcleos(int ud)
{
	ibConf_t *conf = ibConfigs[ud];
	int eosmode;

	if(conf->eos || conf->eosflags)
	{
		eosmode = conf->eos | (conf->eosflags << 8);
	}else
	{
		eosmode = ibBoard[conf->board].eos | (ibBoard[conf->board].eosflags << 8);
	}
	return ibBoardFunc(conf->board, IBEOS, eosmode);
}

