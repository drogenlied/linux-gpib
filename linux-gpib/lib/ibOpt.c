
#include <ib.h>
#include <ibP.h>


PUBLIC int ibchbase(int ud, int base)
{

  return ibBoardFunc(CONF(ud,board),CFCBASE,base);

}

PUBLIC int ibchirq(int ud, int base)
{

  return ibBoardFunc(CONF(ud,board),CFCIRQ,base);

}

PUBLIC int ibchdma(int ud, int base)
{

  return ibBoardFunc(CONF(ud,board),CFCDMA,base);

}

