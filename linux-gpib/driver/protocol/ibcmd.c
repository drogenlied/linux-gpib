#include <ibprot.h>



/*
 * IBCMD
 * Write cnt command bytes from buf to the GPIB.  The
 * command operation terminates only on I/O complete.
 *
 * NOTE:
 *      1.  Prior to beginning the command, the interface is
 *          placed in the controller active state.
 *      2.  Before calling ibcmd for the first time, ibsic
 *          must be called to initialize the GPIB and enable
 *          the interface to leave the controller idle state.
 */
IBLCL ssize_t ibcmd(uint8_t *buf, size_t length)
{
	size_t	count = 0;
	ssize_t ret = 0;
	int status = ibstatus();

	if((status & CIC) == 0)
	{
		printk("gpib: cannot send command when not controller-in-charge\n");
		return -1;
	}
	// XXX global
	osStartTimer(timeidx);

	if((ret = driver->take_control(driver, 0)))
	{
		printk("gpib error while becoming active controller\n");
	}else while ((count < length) &&
		((status = ibstatus()) & TIMO) == 0)
	{
		ret = driver->command(driver, buf, length - count);
		if(ret < 0)
		{
			printk("error writing gpib command bytes\n");
			break;
		}
		buf += ret;
		count += ret;
	}

	osRemoveTimer();

	if(status & TIMO)
		ret = -ETIMEDOUT;

	return count ? count : ret;
}


