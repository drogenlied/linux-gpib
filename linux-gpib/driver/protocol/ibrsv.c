#include <ibprot.h>


/*
 * IBRSV
 * Request service from the CIC and/or set the serial poll
 * status byte.
 */
IBLCL int ibrsv(int v)
{
	int status = board.update_status();

	DBGin("ibrsv");
	if((status & CIC)) 
	{
		DBGout();
		return status;
	}
	DBGprint(DBG_DATA, ("stb=0x%x  ", v));
	bdSetSPMode(v);		/* set new status to v */
	DBGout();
	return status;
}
