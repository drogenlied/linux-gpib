#include <ibprot.h>


/*
 * IBGTS
 * Go to the controller standby state from the controller
 * active state, i.e., turn ATN off.
 */

IBLCL int ibgts(void)
{
	int status = board.update_status();

	DBGin("ibgts");
	if((status & CIC) == 0) 
	{
		DBGout();
		return ibsta;
	}
	board.go_to_standby();                    /* go to standby */
	DBGout();
	return status;
}

