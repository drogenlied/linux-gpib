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
IBLCL ssize_t ibcmd(uint8_t *buf, size_t cnt)
{
	size_t	bytes_sent = 0;
	ssize_t ret = 0;
	int status = driver->update_status();

	if((status & CIC) == 0)
	{
		return -1;
	}
	osStartTimer(timeidx);

	if(driver->take_control(1))
	{
		printk("gpib error while becoming active controller\n");
		return -1;
	}
	while ((bytes_sent < cnt) && !(driver->update_status() & (TIMO)))
	{
		ret = driver->command(buf, cnt);
		if(ret < 0)
		{
			printk("error writing gpib command bytes\n");
			break;
		}
		buf += ret;
		bytes_sent += ret;
	}

	osRemoveTimer();

	if(ret < 0)
		return ret;

	return bytes_sent;
}


