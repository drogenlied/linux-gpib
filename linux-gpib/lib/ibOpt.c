
#include <ib.h>
#include <ibP.h>


int ibchbase(int ud, int base)
{
  return ibBoardFunc(CONF(ud,board),CFCBASE,base);
}

int ibchirq(int ud, int base)
{
  return ibBoardFunc(CONF(ud,board),CFCIRQ,base);
}

int ibchdma(int ud, int base)
{
  return ibBoardFunc(CONF(ud,board),CFCDMA,base);
}

