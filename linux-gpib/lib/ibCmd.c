
#include <ib.h>
#include <ibP.h>

int ibcmd(int ud,char *cmd,unsigned long cnt)
{
  ibtmo(ud, CONF(ud,tmo));
  return ibBoardFunc(CONF(ud,board),IBCMD,0, cmd,cnt);
}
