
#include "ib_internal.h"
#include <ibP.h>

int ibwrt(int ud, void *rd, unsigned long cnt)
{
	ibConf_t *conf = ibConfigs[ud];

	iblcleos(ud);
	ibtmo(ud, conf->tmo);

return  ibBoardFunc( conf->board, (conf->flags & CN_ISCNTL) ? IBWRT : DVWRT,
	padsad(conf->pad, conf->sad), rd, cnt);
}




