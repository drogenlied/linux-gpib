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
	int status = board.update_status();
	if((status * CIC) == 0)
	{
		return -1;
	}
	osStartTimer(pollTimeidx);
	board.take_control(0);
	if(board.parallel_poll(buf)) 
	{
		//XXX handle error
	} 
	board.go_to_standby();             
	if (!noTimo) {
		ibsta |= ERR;              /* something went wrong */
		iberr = EBUS;
	}
	osRemoveTimer();
	return 0;
}









