
#include "ib_internal.h"
#include <ibP.h>

int ibsic(int ud)
{
  return  ibBoardFunc(CONF(ud,board),IBSIC,0);
}

