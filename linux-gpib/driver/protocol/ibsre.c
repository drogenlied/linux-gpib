#include <ibprot.h>
#include <linux/delay.h>

extern int drvstat,ib_opened;
/*
 * IBSRE
 * Send REN true if v is non-zero or false if v is zero.
 */
int ibsre(gpib_device_t *device, int enable)
{
	pgmstat |= PS_SAC;
	device->interface->remote_enable(device, enable);	/* clear REN */
	if(!enable)
	{
		udelay(100);
	}

	return 0;
}

