
#include <ib.h>
#include <ibP.h>

PUBLIC int ibcac(int ud, int v)
{

  return ibBoardFunc( CONF(ud,board), IBCAC, v);

}

