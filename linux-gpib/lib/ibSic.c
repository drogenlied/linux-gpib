
#include <ib.h>
#include <ibP.h>

PUBLIC int ibsic(int ud)
{

  return  ibBoardFunc(CONF(ud,board),IBSIC,0);

}
