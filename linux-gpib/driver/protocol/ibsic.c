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
	if( !(drvstat & DRV_IFC) || (ib_opened <= 1))
	{

		pgmstat |= PS_SAC;
			/* set controller state */
		driver->interface_clear(1);                   /* assert IFC */
		udelay(100);
		driver->interface_clear(0);                   /* clear IFC */
		drvstat |= DRV_IFC;
	}

	return 0;
}

