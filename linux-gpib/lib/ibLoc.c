#include <ib.h>
#include <ibP.h>

	
PUBLIC int ibloc(int ud)
{
  char cmds[256];
  
  if (ud < 0) {
    ibsta = CMPL | ERR;
    iberr = EDVR;
    return ibsta;
  }
  
  cmds[0] = UNL;
  cmds[1] = (LAD | CONF(ud,padsad)) & 0x0f; //XXX guessed on parenthesis
  cmds[2] = GTL;
  cmds[3] = UNL;
  return ibcmd(CONF(ud,board), cmds, 4);
    
} /* ibloc */
