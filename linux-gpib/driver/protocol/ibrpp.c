#include <ibprot.h>


/*
 * IBRPP
 * Conduct a parallel poll and return the byte in buf.
 *
 * NOTE:
 *      1.  Prior to conducting the poll the interface is placed
 *          in the controller active state.
 */
IBLCL int ibrpp(faddr_t buf)
{
	DBGin("ibrpp");
#if 0
	if (fnInit(HR_CIC) & ERR) {
		DBGout();
		return ibsta;
	}
	osStartTimer(pollTimeidx);
	if ( bdGetAdrStat() & HR_NATN) {       /* if standby, go to CAC */
		DBGprint(DBG_BRANCH, ("take control  "));
		bdSendAuxCmd(AUX_TCA);
		bdWaitOut();
		/* make sure everyone is ready */
	}
	bdSendAuxCmd(AUX_EPP);            /* execute parallel poll */
	DBGprint(DBG_BRANCH, ("wait for CO  "));
	bdWaitOut();
		                           /* wait for poll to complete */
	if (!noTimo) {
		ibsta |= ERR;              /* something went wrong */
		iberr = EBUS;
	}
	*buf = bdGetCmdByte();                /* store the response byte */
	DBGprint(DBG_DATA, ("pp=0x%x  ", *buf));
	osRemoveTimer();
	ibstat();
#endif
	DBGout();
	return ibsta;
}









