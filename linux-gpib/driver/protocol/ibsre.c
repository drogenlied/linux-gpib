#include <ibprot.h>

extern int drvstat,ib_opened;
/*
 * IBSRE
 * Send REN true if v is non-zero or false if v is zero.
 */
IBLCL int ibsre(int v)
{
	DBGin("ibsre");

        if( !(drvstat & DRV_REN || !v ) || (ib_opened <= 1) ){
#if 0
	  if (fnInit(0) & ERR) {
	    DBGout();
	    return ibsta;
	  }
#endif
	  pgmstat |= PS_SAC;
	board.remote_enable(v);	/* set or clear REN */
          if( !v ) drvstat &= ~DRV_REN;
          else     drvstat |= DRV_REN;
        }
	  
	DBGout();
	return board.update_status();
}

