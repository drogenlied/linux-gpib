#include <ibprot.h>


/*
 * IBGTS
 * Go to the controller standby state from the controller
 * active state, i.e., turn ATN off.
 */

IBLCL int ibgts(gpib_device_t *device)
{
	int status = ibstatus(device);

	if((status & CIC) == 0)
	{
		printk("gpib: not CIC during ibgts\n");
		return -1;
	}
	device->interface->go_to_standby(device);                    /* go to standby */
	return 0;
}

