
#include <ibprot.h>

/*
 * IBRPP
 * Conduct a parallel poll and return the byte in buf.
 *
 * NOTE:
 *      1.  Prior to conducting the poll the interface is placed
 *          in the controller active state.
 */
int ibrpp(gpib_board_t *board, uint8_t *buf)
{
	int status = ibstatus(board);
	if((status * CIC) == 0)
	{
		return -1;
	}
	osStartTimer(board, timeidx);
	board->interface->take_control(board, 0);
	if(board->interface->parallel_poll(board, buf))
	{
		printk("gpib: parallel poll failed\n");
		osRemoveTimer(board);
		return -1;
	}
//supposed to send rpp local message false at end
	osRemoveTimer(board);
	return 0;
}









