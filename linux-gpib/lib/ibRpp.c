

#include <ib.h>
#include <ibP.h>

PUBLIC int ibrpp(int ud, char *ppr)
{

return ibBoardFunc(CONF(ud,board),IBRPP,0 ,ppr);

}
