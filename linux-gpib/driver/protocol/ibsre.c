#include <ibprot.h>
#include <linux/delay.h>

extern int drvstat,ib_opened;
/*
 * IBSRE
 * Send REN true if v is non-zero or false if v is zero.
 */
IBLCL int ibsre(gpib_device_t *device, int enable)
{
	pgmstat |= PS_SAC;
	device->interface->remote_enable(device, enable);	/* clear REN */
	if(enable)
	{
		drvstat |= DRV_REN;
	}else
	{
		drvstat &= ~DRV_REN;
		udelay(100);
	}

	return 0;
}

