#include <ibprot.h>


/*
 * IBCAC
 * Return to the controller active state from the
 * controller standby state, i.e., turn ATN on.  Note
 * that in order to enter the controller active state
 * from the controller idle state, ibsic must be called.
 * If v is non-zero, take control synchronously, if
 * possible.  Otherwise, take control asynchronously.
 */
IBLCL int ibcac(int v)
{
	int status = board.update_status();
	DBGin("ibcac");
	if((status & CIC) == 0) 
	{
		DBGout();
		return status;
	}
	if (v && (pgmstat & PS_HELD) && (status & LACS)) {
		DBGprint(DBG_BRANCH, ("sync  "));
		board.take_control(1);            /* assert ATN synchronously */
	}
	else {
		DBGprint(DBG_BRANCH, ("async  "));
		board.take_control(0);            /* assert ATN asynchronously */
	}
	DBGout();
	return status;
}




