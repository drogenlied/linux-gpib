#include <ibprot.h>


/*
 * IBRPP
 * Conduct a parallel poll and return the byte in buf.
 *
 * NOTE:
 *      1.  Prior to conducting the poll the interface is placed
 *          in the controller active state.
 */
int ibrpp(gpib_device_t *device, uint8_t *buf)
{
	int status = ibstatus(device);
	if((status * CIC) == 0)
	{
		return -1;
	}
	osStartTimer(device, pollTimeidx);
	device->interface->take_control(device, 0);
	if(device->interface->parallel_poll(device, buf))
	{
		printk("gpib: parallel poll failed\n");
		return -1;
	}
//supposed to send rpp local message false at end
	osRemoveTimer(device);
	return 0;
}









