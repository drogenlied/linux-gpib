
#include <ib.h>
#include <ibP.h>

int ibrsv(int ud, int v)
{
return  ibBoardFunc(CONF(ud,board),IBRSV,v);
}

