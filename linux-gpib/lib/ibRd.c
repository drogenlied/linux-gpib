
#include <ib.h>
#include <ibP.h>

PUBLIC int ibrd(int ud, char *rd, unsigned long cnt)
{

  char cmd[5];  
  char *adr;
  int  stat;

  iblcleos(ud);
  ibtmo(ud, CONF(ud,tmo));

  return ibBoardFunc(  CONF(ud,board),
                       ( CONF(ud,flags) & CN_ISCNTL  ? IBRD : DVRD ),
                       CONF(ud,padsad), rd, cnt);

}




