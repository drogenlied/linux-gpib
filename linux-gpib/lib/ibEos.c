
#include <ib.h>
#include <ibP.h>

int ibeos(int ud, int v)
{

return ibBoardFunc(CONF(ud,board), IBEOS, v);

}

/*
 *  iblcleos()
 *  sets the eos modes of the unit description (local) or
 *  board description (if none)
 *
 */

int iblcleos(int ud)
{

  if( CONF(ud,eos) | CONF(ud,eosflags) ){  /* set only if unit description has been changed*/
    return ibBoardFunc(CONF(ud,board),IBEOS, CONF(ud,eos) | (CONF(ud,eosflags) << 8 ));
  }
  else
    return ibBoardFunc(CONF(ud,board),IBEOS,ibBoard[CONF(ud,board)].eos 
		 | (ibBoard[CONF(ud,board)].eosflags << 8) );

}

