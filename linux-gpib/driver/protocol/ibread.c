
#include <ibprot.h>


/*
 * IBRD
 * Read up to cnt bytes of data from the GPIB into buf.  End
 * on detection of EOI.
 *
 * NOTE:
 *      1.  The interface is placed in the controller standby
 *          state prior to beginning the read.
 *      2.  Prior to calling ibrd, the intended devices as well
 *          as the interface board itself must be addressed by
 *          calling ibcmd.
 */

IBLCL int ibrd(faddr_t buf, unsigned int cnt)
{
	ibio_op_t	rdop;
	unsigned int	requested_cnt;

	DBGin("ibrd");
	if (fnInit(HR_LA) & ERR) {
		ibcnt = 0;
		DBGout();
		return ibsta;
	}

	DBGprint(DBG_BRANCH, ("go to standby  "));
	bdSendAuxCmd(AUX_GTS);	/* if CAC, go to standby */
	osStartTimer(timeidx);
	requested_cnt = cnt;
	rdop.io_vbuf = buf;
	rdop.io_flags = IO_READ;
	while ((cnt > 0) && !(ibsta & (ERR | TIMO | END))) {
		ibcnt = 0;
		rdop.io_cnt = cnt;
		bdAdjCnt(&rdop);
		bdread(&rdop);
		rdop.io_vbuf += ibcnt;
		cnt -= ibcnt;
	}
	ibcnt = requested_cnt - cnt;
	osRemoveTimer();

	if ((pgmstat & PS_NOEOSEND) && (ibsta & END)) {
		if (! bdCheckEOI() )
			ibsta &= ~END;
	}
	ibstat();
	DBGout();
	return ibsta;
}

