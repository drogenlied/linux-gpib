
#include <ib.h>
#include <ibP.h>

int ibape(int ud, int v)
{
  return ibBoardFunc(CONF(ud,board),IBAPE,CONF(ud,padsad),(char *)NULL, v );
}
