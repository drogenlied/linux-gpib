#include <ibprot.h>
#include <linux/delay.h>

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
	DBGin("ibsic");
	if( !(drvstat & DRV_IFC) || (ib_opened <= 1)){ 
	if (fnInit(0) & ERR) {
		DBGout();
		return ibsta;
	}
	pgmstat |= PS_SAC;
        /* set controller state */
	bdsc();
	board.interface_clear(1);                   /* assert IFC */
	udelay(100);
	board.interface_clear(0);                   /* clear IFC */
	drvstat |= DRV_IFC;
        }
	ibstat();
#if defined(CBI_4882)
	 fix4882Bug();

#endif
	DBGout();
	return ibsta;
}

