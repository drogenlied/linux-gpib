
#include <ib.h>
#include <ibP.h>

int ibpad(int ud, int v)
{
   if (CONF(ud,flags) & CN_ISCNTL)
      return ibBoardFunc(CONF(ud,board),IBPAD,v);
   /* enable ibpad also working on devices, not only on boards */
   else
   {
      if (v >= 31)
      {
         ibsta = CMPL | ERR;
         iberr = EARG;
         ibcnt = errno;
         ibPutErrlog(-1,ibVerbCode(IBPAD));
      }
      else
      {
         ibConfigs[ud]->padsad &= 0xffffff00 ;
         ibConfigs[ud]->padsad |= v ;
      }
   }
	return 0;	//XXX
}
