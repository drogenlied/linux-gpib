#include <ibprot.h>


/*
 * IBLINES
 * Poll the GPIB control lines and return their status in buf.
 *
 *      LSB (bits 0-7)  -  VALID lines mask (lines that can be monitored).
 * Next LSB (bits 8-15) - STATUS lines mask (lines that are currently set).
 *
 */
IBLCL int iblines(int *buf)
{
	if(driver->line_status == NULL)
	{
		printk("driver cannot query gpib line status\n");
		return -1;
	}
	*buf = driver->line_status(driver);
	return 0;
}
