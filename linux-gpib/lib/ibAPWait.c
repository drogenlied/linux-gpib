
#include <ib.h>
#include <ibP.h>

int ibAPwait(int ud, int mask)
{
char spr;


if ( mask & RQS && !(ud & UD_REMOTE) ){

  while(1){
  /* wait for SRQ */
  if( ibBoardFunc(CONF(ud,board),IBAPWAIT, ( mask | SRQI ) & ~RQS ) & ( ERR | TIMO )) 
       return ERR;  
  /* Serial Poll Device */
  if(  ibBoardFunc( CONF(ud,board),
                     IBAPRSP,
                     CONF(ud,padsad), &spr ) & ( ERR | TIMO )) 
        return ERR;
  /* if RQS set return */
  if ( spr & 0x40 ) return spr;
  }

}
else
  return  ibBoardFunc(CONF(ud,board),IBWAIT, mask);



}
