#include <ibprot.h>


/*
 * IBGTS
 * Go to the controller standby state from the controller
 * active state, i.e., turn ATN off.
 */

IBLCL int ibgts(void)
{
	DBGin("ibgts");
	if (fnInit(HR_CIC) & ERR) {
		DBGout();
		return ibsta;
	}
	board.go_to_standby();                    /* go to standby */
	ibstat();
	DBGout();
	return ibsta;
}

