
#include "ib_internal.h"
#include <ibP.h>

int ibcac(int ud, int v)
{
  return ibBoardFunc( CONF(ud,board), IBCAC, v);
}
