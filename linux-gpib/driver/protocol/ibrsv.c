#include <ibprot.h>


/*
 * IBRSV
 * Request service from the CIC and/or set the serial poll
 * status byte.
 */
int ibrsv(gpib_board_t *board, uint8_t poll_status)
{
	int status = ibstatus(board);

	if((status & CIC))
	{
		printk("gpib: interface requested service while CIC\n");
		return -1;
	}
	board->interface->serial_poll_response(board, poll_status);		/* set new status to v */
	return 0;
}
