#include <ibprot.h>


/*
 * IBWAIT
 * Check or wait for a GPIB event to occur.  The mask argument
 * is a bit vector corresponding to the status bit vector.  It
 * has a bit set for each condition which can terminate the wait
 * If the mask is 0 then
 * no condition is waited for and the current status is simply
 * returned.
 */
IBLCL int ibwait(unsigned int mask)
{
	DBGin("ibwait");
	if (mask == 0) {
		DBGprint(DBG_BRANCH, ("mask=0  "));
		DBGout();
		return board.update_status();
	}
	else if (mask & ~WAITBITS) {
		DBGprint(DBG_BRANCH, ("bad mask 0x%x ",mask));
		ibsta |= ERR;
		iberr = EARG;
		DBGout();
		return board.update_status();
	}
	osStartTimer(timeidx);
	board.wait(mask | TIMO);
	if (!noTimo)
		ibsta |= TIMO;
	osRemoveTimer();
	DBGout();
	return board.update_status();
}

