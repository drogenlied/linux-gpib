#include <ibprot.h>


/*
 * IBRSV
 * Request service from the CIC and/or set the serial poll
 * status byte.
 */
IBLCL int ibrsv(gpib_device_t *device, uint8_t poll_status)
{
	int status = ibstatus(device);

	if((status & CIC))
	{
		printk("gpib: interface requested service while CIC\n");
		return -1;
	}
	device->interface->serial_poll_response(device, poll_status);		/* set new status to v */
	return 0;
}
