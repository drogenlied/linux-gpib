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
IBLCL int ibwrt(uint8_t *buf, size_t cnt, unsigned int more)
{
	unsigned int	requested_cnt;
	ssize_t ret;

	DBGin("ibwrt");
	if (fnInit(HR_TA) & ERR) {
		ibcnt = 0;
		DBGout();
		return board.status;
	}
	board.go_to_standby();
	osStartTimer(timeidx);
	requested_cnt = cnt;
	while ((cnt > 0) && !(board.status & (ERR | TIMO))) {
		ret = board.write(buf, cnt, !more);
		if(ret < 0)
		{
			printk("gpib write error\n");
			break;
		}
		buf += ret;
		cnt -= ret;
	}
	ibcnt = requested_cnt - cnt;
	osRemoveTimer();

	ibstat();
	DBGout();
	return ibsta;
}
