#include <ibprot.h>


/*
 * IBRSV
 * Request service from the CIC and/or set the serial poll
 * status byte.
 */
IBLCL int ibrsv(uint8_t poll_status)
{
	int status = board.update_status();

	if((status & CIC)) 
	{
		printk("gpib: interface requested service while CIC\n");
		return -1;
	}
	board.serial_poll_response(poll_status);		/* set new status to v */
	return 0;
}
