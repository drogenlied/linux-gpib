
#include <ib.h>
#include <ibP.h>

PUBLIC int ibpad(int ud, int v)
{
#if NO_RSD_PATCH
  return ibBoardFunc(CONF(ud,board),IBPAD,v);
#else
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
         /* remove network information of ud ???? */
         ud &= 0x0ff ;  
         ibConfigs[ud].padsad &= 0xffffff00 ;
         ibConfigs[ud].padsad |= v ;
      }
   }
#endif
	return 0;	//XXX
}
