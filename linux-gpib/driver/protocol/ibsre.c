#include <ibprot.h>

extern int drvstat,ib_opened;
/*
 * IBSRE
 * Send REN true if v is non-zero or false if v is zero.
 */
IBLCL int ibsre(gpib_device_t *device, int enable)
{
	pgmstat |= PS_SAC;
	device->interface->remote_enable(device, enable);	/* set or clear REN */
	if( !enable ) drvstat &= ~DRV_REN;
	else drvstat |= DRV_REN;

	return 0;
}

