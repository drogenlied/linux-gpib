#include <ibprot.h>


/*
 * IBWRT
 * Write cnt bytes of data from buf to the GPIB.  The write
 * operation terminates only on I/O complete.  By default,
 * EOI is always sent along with the last byte.  'more' is
 * a boolean value indicating that there remains more data
 * that will be written as a part of this output (so don't
 * send EOI/EOS).
 *
 * NOTE:
 *      1.  Prior to beginning the write, the interface is
 *          placed in the controller standby state.
 *      2.  Prior to calling ibwrt, the intended devices as
 *          well as the interface board itself must be
 *          addressed by calling ibcmd.
 *      3.  Be sure to type cast the buffer to (faddr_t) before
 *          calling this function.
 */
IBLCL int ibwrt(faddr_t buf, unsigned int cnt, unsigned int more)
{
	ibio_op_t	wrtop;
	unsigned int	requested_cnt;

	DBGin("ibwrt");
	if (fnInit(HR_TA) & ERR) {
		ibcnt = 0;
		DBGout();
		return ibsta;
	}
	osStartTimer(timeidx);
	DBGprint(DBG_BRANCH, ("go to standby  "));
	bdSendAuxCmd(AUX_GTS);	/* if CAC, go to standby */
	requested_cnt = cnt;
	wrtop.io_vbuf = buf;
	wrtop.io_flags = 0;
	if(more == 0) wrtop.io_flags |= IO_LAST;
	while ((cnt > 0) && !(ibsta & (ERR | TIMO))) {
		ibcnt = 0;
		wrtop.io_cnt = cnt;
		bdwrt(&wrtop);
		wrtop.io_vbuf += ibcnt;
		cnt -= ibcnt;
	}
	ibcnt = requested_cnt - cnt;
	osRemoveTimer();

	ibstat();
	DBGout();
	return ibsta;
}
