#include <ibprot.h>


/*
 * IBLINES
 * Poll the GPIB control lines and return their status in buf.
 *
 *      LSB (bits 0-7)  -  VALID lines mask (lines that can be monitored).
 * Next LSB (bits 8-15) - STATUS lines mask (lines that are currently set).
 *
 */
IBLCL int iblines(int *buf)
{
	int status = board.update_status();

	DBGin("iblines");
//	if (fnInit(0) & ERR)
//		*buf = 0;
//	else {
	*buf = bdlines();
	ibstat();
//	}
	DBGout();
	return status;
}
