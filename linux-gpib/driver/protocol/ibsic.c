#include <ibprot.h>


extern int drvstat,ib_opened;

/*
 * IBSIC
 * Send IFC for at least 100 microseconds.
 *
 * NOTE:
 *      1.  Ibsic must be called prior to the first call to
 *          ibcmd in order to initialize the bus and enable the
 *          interface to leave the controller idle state.
 */
IBLCL int ibsic(void)
{
	int	i;

	DBGin("ibsic");
	if( !(drvstat & DRV_IFC) || (ib_opened <= 1)){ 
	if (fnInit(0) & ERR) {
		DBGout();
		return ibsta;
	}
	pgmstat |= PS_SAC;
        /* set controller state */
	bdsc();
	bdSendAuxCmd(AUX_SIFC);                   /* assert IFC */
	for(i = 0; i < ifcDelay; i++);              /* busy wait >= ~100us */
	bdSendAuxCmd(AUX_CIFC);                   /* clear IFC */
	drvstat |= DRV_IFC;
        }
	ibstat();
#if defined(CBI_4882)
	 fix4882Bug();

#endif
	DBGout();
	return ibsta;
}

