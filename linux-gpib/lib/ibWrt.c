
#include <ib.h>
#include <ibP.h>

PUBLIC int ibwrt(int ud, char *rd, unsigned long cnt)
{

  char cmd[5];  
  char *adr;

  iblcleos(ud);
  ibtmo(ud, CONF(ud,tmo));


  return  ibBoardFunc(  CONF(ud,board),
		        (CONF(ud,flags) & CN_ISCNTL ? IBWRT : DVWRT ),
                        CONF(ud,padsad), 
                        rd, cnt);

}




