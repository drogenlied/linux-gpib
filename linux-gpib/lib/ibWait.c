
#include "ib_internal.h"
#include <ibP.h>

int ibwait(int ud, int mask)
{
char spr;
int pollflag = 0;
int amask;

if( CONF(ud,flags) & CN_AUTOPOLL ) pollflag=1;

if ( mask & RQS ){
  amask = ( mask | SRQI ) & ~RQS ;

  while(1){
  /* wait for SRQ */ 
  if( ibBoardFunc(CONF(ud,board),
		 (pollflag?IBAPWAIT:IBWAIT),
		 (pollflag? CONF(ud,padsad)&0xff :amask)) & ( ERR | TIMO ) ) 
       return ERR;  
  /* Serial Poll Device */
  if(  ibBoardFunc( CONF(ud,board),
                     (pollflag?IBAPRSP:DVRSP),
                     CONF(ud,padsad), &spr ) & ( ERR | TIMO )) 
        return ERR;
  /* if RQS set return */
  if ( spr & 0x40 ) return spr;
  }

}
else
  return  ibBoardFunc(CONF(ud,board),IBWAIT, mask); /*pollflag not necessary will be*/ 
                                                    /*taken from remote*/

}
