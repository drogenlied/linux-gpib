#include <ibprot.h>


/*
 * IBRSV
 * Request service from the CIC and/or set the serial poll
 * status byte.
 */
IBLCL int ibrsv(uint8_t poll_status)
{
	int status = driver->update_status(driver);

	if((status & CIC))
	{
		printk("gpib: interface requested service while CIC\n");
		return -1;
	}
	driver->serial_poll_response(driver, poll_status);		/* set new status to v */
	return 0;
}
