#include <ibprot.h>

extern int drvstat,ib_opened;
/*
 * IBSRE
 * Send REN true if v is non-zero or false if v is zero.
 */
IBLCL int ibsre(int enable)
{
	pgmstat |= PS_SAC;
	driver->remote_enable(enable);	/* set or clear REN */
	if( !enable ) drvstat &= ~DRV_REN;
	else drvstat |= DRV_REN;

	return 0;
}

