#include <stdio.h>

#include <ib.h>
#include <ibP.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/ioctl.h>

int iberr = 0;
int ibsta = 0;
int ibcnt = 0;

ibarg_t ibarg = {0 };


ibBoard_t ibBoard[MAX_BOARDS];

PRIVATE void ibBoardDefaultValues(void)
{
register int i;
for(i=0;i<MAX_BOARDS;i++){

	ibBoard[i].padsad =0;					
	ibBoard[i].eos='\n';
	ibBoard[i].eosflags=0; 
	ibBoard[i].base=0;
	ibBoard[i].irq=0;
	ibBoard[i].dma=0;
	ibBoard[i].ifc=0;
	ibBoard[i].fileno=0;
	ibBoard[i].debug=0;
	ibBoard[i].dmabuf=0;
	strcpy(ibBoard[i].device,"");
        strcpy(ibBoard[i].errlog,"");
      }
}

/**********************/
PRIVATE int ibBoardOpen(int bd,int flags)
{

int fd;

if( ibBoard[bd].fileno == 0 ){

  if( ( fd = open( ibBoard[bd].device, O_RDWR | flags )) < 0 ){

    ibsta =  ERR;
    iberr = EDVR;
    ibcnt = errno;
    ibPutErrlog(-1,"ibBoardOpen");
    return ERR;
  }
  ibBoard[bd].fileno = fd;
}
return 0;
}

/**********************/
PRIVATE int ibBoardClose(int bd)
{

if( ibBoard[bd].fileno > 0 ){
  close( ibBoard[bd].fileno );
  ibBoard[bd].fileno = 0; 
}
return 0;
}

/**********************/

PRIVATE int ibBoardFunc (int bd, int code, ...)
{
  va_list ap;
  static int arg;
  static char *buf;
  static int cnt;

  switch (code) {
  case IBRD:
  case DVRD:
  case IBWRT:
  case DVWRT:
  case IBAPE:
  case IBCMD:
    va_start(ap, code);
    arg=va_arg(ap, int);
    buf=va_arg(ap, char *);
    cnt=va_arg(ap, int);
    va_end(ap);
    break;
  case IBAPRSP:
  case IBRPP:
  case DVRSP:
    va_start(ap, code);
    arg=va_arg(ap, int);
    buf=va_arg(ap, char *);
    buf[0]=0;
    va_end(ap);
    cnt=1;
    break;
  default:
    va_start(ap, code);
    arg=va_arg(ap, int);
    va_end(ap);
    buf=NULL;
    cnt=0;
    break;
  }

#if HAS_RGPIB
  if( bd & UD_REMOTE ) {  /* board handle is a network request handle */
      /* do request */
      return ibRemoteFunc(bd,code,arg,buf,cnt);
  } 
  /**** local device operation ***/
#endif

  switch (code) {
  case IBRD:
  case DVRD:
    memset(buf,0,cnt);
    break;
  }

  if( ibBoard[bd].fileno > 0 ){
    ibarg.ib_arg = arg;

    ibarg.ib_buf = (char *) buf;

    ibarg.ib_cnt = cnt;
    if( ioctl( ibBoard[bd].fileno, code, (ibarg_t *) &ibarg ) < 0){
      ibsta = ERR;
      iberr = EDVR;
      ibcnt = errno;
    } else {
      ibsta = ibarg.ib_ibsta;
      iberr = ibarg.ib_iberr;
      ibcnt = ibarg.ib_ibcnt;
      ibPutErrlog(-1,ibVerbCode(code));
    }
  } else {
    ibsta = CMPL | ERR;
    iberr = ENEB;
    ibcnt = 0;
  } 
  /* manfred sand's patch */
  if(code == IBRD || code==DVRD ) memcpy(buf,ibarg.ib_buf,ibcnt);
  ibPutErrlog(-1,"ibBoardFunc");
  return ibsta;
}


PRIVATE int ibGetNrBoards(void)
{
extern int bdid;
 
 return bdid+1;

}





