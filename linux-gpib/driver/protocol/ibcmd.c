
#include "gpibP.h"

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
ssize_t ibcmd( gpib_board_t *board, uint8_t *buf, size_t length )
{
	size_t	count = 0;
	ssize_t ret = 0;
	int status = ibstatus(board);

	if(length == 0) return 0;

	if((status & CIC) == 0)
	{
		printk("gpib: cannot send command when not controller-in-charge\n");
		return -1;
	}

	osStartTimer( board, board->usec_timeout );

	if((ret = board->interface->take_control(board, 0)))
	{
		printk("gpib error while becoming active controller\n");
		ret = -1;
	}else
	{
		ret = board->interface->command(board, buf, length - count);
		if(ret < 0)
		{
			printk("error writing gpib command bytes\n");
		}else
		{
			buf += ret;
			count += ret;
		}
	}

	osRemoveTimer(board);

	if(status & TIMO)
		ret = -ETIMEDOUT;

	return ret ? ret : count;
}


