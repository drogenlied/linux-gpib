#include <ibprot.h>


/*
 * IBRSV
 * Request service from the CIC and/or set the serial poll
 * status byte.
 */
IBLCL int ibrsv(int v)
{
	DBGin("ibrsv");
	if (fnInit(0) & ERR) {
		DBGout();
		return ibsta;
	}
	DBGprint(DBG_DATA, ("stb=0x%x  ", v));
	bdSetSPMode(v);		/* set new status to v */
	ibstat();
	DBGout();
	return ibsta;
}
