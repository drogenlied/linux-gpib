
#include <ib.h>
#include <ibP.h>

PUBLIC int ibgts(int ud, int v)
{

return ibBoardFunc(CONF(ud,board), IBGTS, v);

}
