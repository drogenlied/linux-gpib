#include <ibprot.h>
#include <linux/kernel.h>

int pgmstat	= 0;		/* program status vector */

int timeidx	= DFLTTIMO;	/* timeout index into timeTable */
int pollTimeidx	= T100ms;	/* timeidx for serial and parallel polls */
int myPAD		= PAD;		/* current primary address */
int mySAD		= SAD;		/* current secondary address */

#if !defined(HP82335) && !defined(TMS9914)
#else
int ccrbits	= 0;	/* static bits for AUXRA (EOS modes) */
#endif

uint32_t timeTable[] = {
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

IBLCL int ibonl(gpib_device_t *device, int v)
{
	/*
	* ibonl must be called first time a process is entering the driver
	* if one process is working on the driver ibonl is dummied
	*
	*/
	if( ib_opened == 1 || !(drvstat & DRV_ONLINE) )
	{
		pgmstat &= PS_STATIC; /* initialize program status vector */
		timeidx = DFLTTIMO; /* initialize configuration variables... */
		myPAD = PAD;
		mySAD = SAD;
#if !defined(HP82335) && !defined(TMS9914)
#else
		ccrbits = 0;
#endif
	}

	if (v)
	{
		if( (ib_opened <= 1) && !( drvstat & DRV_ONLINE ))
		{
			if(device->interface->attach(device) < 0)
			{
				device->interface->detach(device);
				printk("GPIB Hardware Error! (Chip type not found or wrong Base Address?)\n");
				return -1;
			}

			if(!osInit())
			{
				printk("GPIB System Request Error! \n");
				return -1;
			}
		}

		/* initialize system support functions */
		pgmstat |= PS_ONLINE;
		drvstat |= DRV_ONLINE;
	}else
	{		/* OFFLINE: leave SYSFAIL red */
		if( ib_opened <= 1)
		{
			DBGprint(DBG_BRANCH,("Board Offline"));
			device->interface->detach(device);
			if (pgmstat & PS_SYSRDY)
			osReset();	/* reset system interface */
			pgmstat &= ~PS_ONLINE;
			drvstat &= ~DRV_ONLINE;
		}
	}
	return 0;
}

