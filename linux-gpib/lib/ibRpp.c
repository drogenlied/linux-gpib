
#include "ib_internal.h"
#include <ibP.h>

int ibrpp(int ud, char *ppr)
{
return ibBoardFunc(CONF(ud,board),IBRPP,0 ,ppr);
}
