
#include "gpibP.h"

/*
 * IBRPP
 * Conduct a parallel poll and return the byte in buf.
 *
 * NOTE:
 *      1.  Prior to conducting the poll the interface is placed
 *          in the controller active state.
 */
int ibrpp(gpib_board_t *board, uint8_t *result )
{
	int status = ibstatus( board );
	int retval = 0;

	if( ( status & CIC ) == 0 )
	{
		return -1;
	}
	osStartTimer( board, board->usec_timeout );
	board->interface->take_control(board, 0);
	if(board->interface->parallel_poll( board, result ) )
	{
		printk("gpib: parallel poll failed\n");
		retval = -1;
	}
	osRemoveTimer(board);
	return retval;
}

int ibppc( gpib_board_t *board, uint8_t configuration )
{

	configuration &= 0x1f;
	board->interface->parallel_poll_response( board, configuration );

	return 0;
}







