
#include <ib.h>
#include <ibP.h>

int ibonl(int ud, int onl)
{
	int oflags=0;
	ibConf_t *conf = ibConfigs[ud];

	if(ibCheckDescriptor(ud) < 0)
	{
		ibsta |= ERR;
		return ibsta;
	}

	if(conf->flags & CN_EXCLUSIVE )
		oflags |= O_EXCL;

	if((ibBoard[conf->board].fileno < 0)
		&& (ibBoardOpen(conf->board, oflags) & ERR))
	{
		ibsta = ibarg.ib_ibsta | ERR;
		iberr = EDVR;
		ibcnt = errno;
		return ibsta;
	}

	ibBoardFunc(conf->board, IBONL, onl );
	if(onl == 0)
	{
		if(conf->is_interface)
			ibBoardClose(conf->board);
	}

	return ibsta;
}


