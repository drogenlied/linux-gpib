
#include <ib.h>
#include <ibP.h>

int ibeot(int ud, int v)
{
return ibBoardFunc(CONF(ud,board), IBEOT, v);
}
