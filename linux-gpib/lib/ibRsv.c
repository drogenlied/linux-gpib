
#include <ib.h>
#include <ibP.h>

PUBLIC int ibrsv(int ud, int v)
{

return  ibBoardFunc(CONF(ud,board),IBRSV,v);

}
