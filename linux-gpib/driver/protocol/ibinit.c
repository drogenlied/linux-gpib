#include <ibprot.h>
#include <linux/kernel.h>

int       ibsta	= 0; 		/* status bits returned by last call      */
int       ibcnt	= 0; 		/* actual byte count of last I/O transfer */
int       iberr	= 0; 		/* error code from last function call     */

int pgmstat	= 0;		/* program status vector */
volatile int noTimo	= INITTIMO;	/* 0 = I/O or wait operation timed out */

int timeidx	= DFLTTIMO;	/* timeout index into timeTable */
int pollTimeidx	= T100ms;	/* timeidx for serial and parallel polls */
int myPAD		= PAD;		/* current primary address */
int mySAD		= SAD;		/* current secondary address */
int ifcDelay	= 200;		/* delay loop counter for IFC pulse */

#if !defined(HP82335) && !defined(TMS9914)
int auxrabits	= AUXRA;	/* static bits for AUXRA (EOS modes) */
#else
int ccrbits	= 0;	/* static bits for AUXRA (EOS modes) */
#endif

uint32 timeTable[] = {
/*
 *	Since timeTable[0] is never used, store the
 *	current timing factor at this location...
 */
           TMFAC,                      	/*  0: TNONE    */
        TM(TMFAC,10,1000000L),        	/*  1: T10us    */
        TM(TMFAC,30,1000000L),        	/*  2: T30us    */
        TM(TMFAC,100,1000000L),       	/*  3: T100us   */
        TM(TMFAC,300,1000000L),       	/*  4: T300us   */
        TM(TMFAC,1,1000),             	/*  5: T1ms     */
        TM(TMFAC,3,1000),             	/*  6: T3ms     */
        TM(TMFAC,10,1000),            	/*  7: T10ms    */
        TM(TMFAC,30,1000),            	/*  8: T30ms    */
        TM(TMFAC,100,1000),           	/*  9: T100ms   */
        TM(TMFAC,300,1000),           	/* 10: T300ms   */
        TM(TMFAC,1,1),                	/* 11: T1s      */
        TM(TMFAC,3,1),                	/* 12: T3s      */
        TM(TMFAC,10,1),               	/* 13: T10s     */
        TM(TMFAC,30,1),               	/* 14: T30s     */
        TM(TMFAC,100,1),              	/* 15: T100s    */
        TM(TMFAC,300,1),              	/* 16: T300s    */
        TM(TMFAC,1000,1)              	/* 17: T1000s   */
};


/*
 * IBONL
 * Initialize the interface hardware.  If v is non-zero then
 * the GPIB chip is enabled online.  If v is zero then it is
 * left disabled and offline.
 *
 * NOTE:
 *      1.  Ibonl must be called before any other function.
 */

extern int ib_opened;

int drvstat = 0;

IBLCL int ibonl(int v)
{
	    DBGin("ibonl");

	    /*
	     * ibonl must be called first time a process is entering the driver
	     * if one process is working on the driver ibonl is dummied
	     *
	     */
	    DBGprint(DBG_DATA,("ib_opened=%d drvstat=0x%x\n",ib_opened,drvstat));
	    if( ib_opened == 1 || !(drvstat & DRV_ONLINE) ){
	      pgmstat &= PS_STATIC; /* initialize program status vector */
	      timeidx = DFLTTIMO; /* initialize configuration variables... */
	      noTimo = INITTIMO; /* initialize timeout flag */
	      myPAD = PAD;
	      mySAD = SAD;
#if !defined(HP82335) && !defined(TMS9914)
	      auxrabits = AUXRA;
#else
	      ccrbits = 0;
#endif
	      ibsta = CMPL;
	      ibcnt = 0;
	    }

	    if (v) {
	      if( (ib_opened <= 1) && !( drvstat & DRV_ONLINE )){
                DBGprint(DBG_BRANCH,("Board Online"));

		if(board_attach() < 0)
		{
			board_detach();
			printk("GPIB Hardware Error! (Chip type not found or wrong Base Address?)\n");
			ibsta |= ERR;
			iberr = ENEB;
			DBGout();
			return ibsta;
		}

		if(! osInit()){
		  printk("GPIB System Request Error! \n");
		  ibsta |= ERR;
		  iberr = ENEB;
		  DBGout();
		  return ibsta;
		}
	      } else {
		DBGprint(DBG_BRANCH,("Board already Online"));
	      }
		/* initialize system support functions */
		pgmstat |= PS_ONLINE;
		drvstat |= DRV_ONLINE;
		ibstat();
	    }
	    else {		/* OFFLINE: leave SYSFAIL red */
              if( ib_opened <= 1) {
		DBGprint(DBG_BRANCH,("Board Offline"));
		board_detach();
		if (pgmstat & PS_SYSRDY)
		  osReset();	/* reset system interface */
		pgmstat &= ~PS_ONLINE;
		drvstat &= ~DRV_ONLINE;
	      }
	    }
	    DBGout();

	    return ibsta;
}

