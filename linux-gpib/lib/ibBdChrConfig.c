#include <ib.h>
#include <ibP.h>

PUBLIC int ibBdChrConfig(int ud, int base,int irq,int dma, int dmabuf)
{


  if( ibBoardOpen( CONF(ud,board) ,0) & ERR ){
    ibsta = ibarg.ib_ibsta | ERR;
    iberr = EDVR;
    ibcnt = errno;
    ibPutErrlog(ud,"ibBdChrConfig");
  } else {

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


