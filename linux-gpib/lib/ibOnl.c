
#include <ib.h>
#include <ibP.h>

PUBLIC int ibonl(int ud, int onl)
{
  extern char ibfind_called;
  int oflags=0;

  if ( ud == ERR )
    return ibsta;

  if( CONF(ud,flags) & CN_ISCNTL || CONF(ud,flags) & CN_EXCLUSIVE )
    oflags |= O_EXCL;


  if ( (ibBoard[ CONF(ud,board) ].fileno <=0) 
      && ( ibBoardOpen( CONF(ud,board),oflags ) & ERR ) ){
          ibsta = ibarg.ib_ibsta | ERR;
	  iberr = EDVR;
	  ibcnt = errno;
  } else {
    ibBoardFunc( CONF(ud,board), IBONL, onl );
    if(!(ibsta & ERR) && !onl){
      ibBoardClose( CONF(ud,board) );
      ibfind_called=0;
    }
  }

  return ibsta;
}


