
#include "gpibP.h"

/*
 * IBLINES
 * Poll the GPIB control lines and return their status in buf.
 *
 *      LSB (bits 0-7)  -  VALID lines mask (lines that can be monitored).
 * Next LSB (bits 8-15) - STATUS lines mask (lines that are currently set).
 *
 */
int iblines(gpib_board_t *board, int *buf)
{
	if(board->interface->line_status == NULL)
	{
		printk("driver cannot query gpib line status\n");
		return -1;
	}
	*buf = board->interface->line_status(board);
	return 0;
}
