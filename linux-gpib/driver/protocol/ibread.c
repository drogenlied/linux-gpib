
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

IBLCL int ibrd(uint8_t *buf, unsigned int cnt)
{
	ibio_op_t	rdop;
	unsigned int	requested_cnt;
	ssize_t ret;

	DBGin("ibrd");
	if (fnInit(HR_LA) & ERR) {
		ibcnt = 0;
		DBGout();
		return ibsta;
	}

	DBGprint(DBG_BRANCH, ("go to standby  "));
	board.go_to_standby();
	osStartTimer(timeidx);
	requested_cnt = cnt;
	while ((cnt > 0) && !(ibsta & (ERR | TIMO | END))) {
		ret = board.read(buf, cnt, 0);	// eos XXX
		if(ret < 0)
		{
			printk("gpib read error\n");
			break;
			// XXX
		}
		buf += ret;
		cnt -= ret;
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

