
#include <ib.h>
#include <ibP.h>

PUBLIC int ibeot(int ud, int v)
{

return ibBoardFunc(CONF(ud,board), IBEOT, v);

}
