#include <ibprot.h>


/*
 * IBRPP
 * Conduct a parallel poll and return the byte in buf.
 *
 * NOTE:
 *      1.  Prior to conducting the poll the interface is placed
 *          in the controller active state.
 */
IBLCL int ibrpp(uint8_t *buf)
{
	int status = driver->update_status();
	if((status * CIC) == 0)
	{
		return -1;
	}
	osStartTimer(pollTimeidx);
	driver->take_control(0);
	if(driver->parallel_poll(buf)) 
	{
		printk("gpib: parallel poll failed\n");
		return -1;
	}
//supposed to send rpp local message false at end
	osRemoveTimer();
	return 0;
}









