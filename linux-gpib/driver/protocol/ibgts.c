
#include <ibprot.h>

/*
 * IBGTS
 * Go to the controller standby state from the controller
 * active state, i.e., turn ATN off.
 */

int ibgts(gpib_board_t *board)
{
	int status = ibstatus(board);

	if((status & CIC) == 0)
	{
		printk("gpib: not CIC during ibgts\n");
		return -1;
	}
	board->interface->go_to_standby(board);                    /* go to standby */
	return 0;
}

