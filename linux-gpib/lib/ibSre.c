
#include "ib_internal.h"
#include <ibP.h>

int ibsre(int ud, int v)
{
return  ibBoardFunc(CONF(ud,board),IBSRE ,v);
}

