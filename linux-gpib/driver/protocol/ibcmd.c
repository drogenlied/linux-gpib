#include <ibprot.h>



/*
 * IBCMD
 * Write cnt command bytes from buf to the GPIB.  The
 * command operation terminates only on I/O complete.
 *
 * NOTE:
 *      1.  Prior to beginning the command, the interface is
 *          placed in the controller active state.
 *      2.  Before calling ibcmd for the first time, ibsic
 *          must be called to initialize the GPIB and enable
 *          the interface to leave the controller idle state.
 *      3.  Be sure to type cast the buffer to (faddr_t) before
 *          calling this function.
 */
IBLCL int ibcmd(faddr_t buf, unsigned int cnt)
{
	ibio_op_t	cmdop;
	unsigned int	requested_cnt;

	DBGin("ibcmd");
	if (fnInit(HR_CIC) & ERR) {
		ibcnt = 0;
		DBGout();
		return ibsta;
	}
	osStartTimer(timeidx);
#if !defined(HP82335) && !defined(TMS9914)
	if ( bdGetAdrStat() & HR_NATN) {	/* if standby, go to CAC */
#else
	if ( !(bdGetAdrStat() & HR_ATN) ) {	/* if standby, go to CAC */
#endif

		DBGprint(DBG_BRANCH, ("take control  "));
		bdSendAuxCmd(AUX_TCA);
		bdWaitOut();
		/*while (!(GPIBin(isr2) & HR_CO) && NotTimedOut())*/
					/* so Turbo488 boards won't jump the gun */
	}
	requested_cnt = cnt;
	cmdop.io_vbuf = buf;
	cmdop.io_flags = 0;
	while ((cnt > 0) && !(ibsta & (ERR | TIMO))) {
		ibcnt = 0;
		cmdop.io_cnt = cnt;
		bdAdjCnt(&cmdop);
		bdcmd(&cmdop);
		cmdop.io_vbuf += ibcnt;
		cnt -= ibcnt;
	}
	ibcnt = requested_cnt - cnt;


	osRemoveTimer();

	ibstat();
	DBGout();
	return ibsta;
}


