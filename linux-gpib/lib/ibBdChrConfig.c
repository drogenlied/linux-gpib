#include <ib.h>
#include <ibP.h>
#include <sys/ioctl.h>
#include <string.h>

PUBLIC int ibBdChrConfig(int ud, int base,int irq,int dma, int dmabuf)
{
	board_type_ioctl_t boardtype;

  if( ibBoardOpen( CONF(ud,board) ,0) & ERR ){
    ibsta = ibarg.ib_ibsta | ERR;
    iberr = EDVR;
    ibcnt = errno;
    ibPutErrlog(ud,"ibBdChrConfig");
  } else {
	strncpy(boardtype.name, ibBoard[CONF(ud, board)].name, 100);
	ioctl(ibBoard[CONF(ud,board)].fileno, CFCBOARDTYPE, &boardtype); 
    if( base != 0 )   ibBoardFunc(CONF(ud,board),CFCBASE, base);
    if( irq  != 0 )   ibBoardFunc(CONF(ud,board),CFCIRQ , irq);
    if( dma  != 0 )   ibBoardFunc(CONF(ud,board),CFCDMA , dma);
    if(dmabuf > 0 ) {
        if( dmabuf % 1024 != 0 ) {
           ibPutMsg(" Size of DMA Buffer not dividable by 1024 ");
           return( ERR );
        }
        else {
           if( ibBoardFunc(CONF(ud,board),CFCDMABUFFER ,dmabuf) & ERR ) {
	     ibPutMsg(" Problem during DMA-Buffer allocation ");
	   }
        }


    }   

    if (!(ibsta & ERR)) {
      iberr = EDVR;
      ibcnt = 0;
      ibPutErrlog(ud,"ibBdChrConfig");
    }
    ibBoardClose( CONF(ud,board) );
  }
  return ibsta;
}


