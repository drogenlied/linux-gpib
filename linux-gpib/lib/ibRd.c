
#include <ib.h>
#include <ibP.h>

int ibrd(int ud, uint8_t *rd, unsigned long cnt)
{
	ibConf_t *conf = ibConfigs[ud];

	if(ibCheckDescriptor(ud) < 0)
		return ibsta | ERR;

	// set eos mode
	iblcleos(ud);
	// set timeout
	ibBoardFunc(conf->board, IBTMO, conf->tmo);

	return ibBoardFunc(conf->board,
		( conf->flags & CN_ISCNTL ? IBRD : DVRD ),
		conf->padsad, rd, cnt);
}




