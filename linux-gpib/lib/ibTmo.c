
#include <ib.h>
#include <ibP.h>

PUBLIC int ibtmo(int ud,int v)
{
    static int current_timeout = -1;

  if ((v < TNONE) || (v > T1000s)) {
    ibsta = CMPL | ERR;
    iberr = EARG;
    return ibsta;
  }

  CONF(ud,tmo) = v;
  if (v != current_timeout) {
    return ibBoardFunc(CONF(ud,board),IBTMO ,v);
  }
  else {
    ibsta = CMPL;
    return ibsta;
  }                

}

