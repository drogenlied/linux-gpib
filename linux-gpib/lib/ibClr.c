
#include "ib_internal.h"
#include <ibP.h>

int ibclr(int ud)
{
	return ibBoardFunc( CONF(ud,board),DVCLR, CONF(ud,padsad));
}
