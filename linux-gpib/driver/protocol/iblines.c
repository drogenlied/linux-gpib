#include <ibprot.h>


/*
 * IBLINES
 * Poll the GPIB control lines and return their status in buf.
 *
 *      LSB (bits 0-7)  -  VALID lines mask (lines that can be monitored).
 * Next LSB (bits 8-15) - STATUS lines mask (lines that are currently set).
 *
 */
int iblines(gpib_device_t *device, int *buf)
{
	if(device->interface->line_status == NULL)
	{
		printk("driver cannot query gpib line status\n");
		return -1;
	}
	*buf = device->interface->line_status(device);
	return 0;
}
