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
IBLCL int ibcmd(uint8_t *buf, size_t cnt)
{
	size_t	requested_cnt;
	ssize_t ret;

	DBGin("ibcmd");
	if (fnInit(HR_CIC) & ERR) {
		ibcnt = 0;
		DBGout();
		return ibsta;
	}
	osStartTimer(timeidx);

	DBGprint(DBG_BRANCH, ("take control  "));
	if(board.take_control(0))
	{
		printk("gpib error while becoming active controller\n");
		return ibsta;
	}
	requested_cnt = cnt;
	while ((cnt > 0) && !(ibsta & (ERR | TIMO))) {
		ret = board.write(buf, cnt, 0);
		if(ret < 0)
		{
			printk("error writing gpib command bytes\n");
			break;
			// XXX
		}
		buf += ret;
		cnt -= ret;
	}
	ibcnt = requested_cnt - cnt;

	board.go_to_standby();

	osRemoveTimer();

	ibstat();
	DBGout();
	return ibsta;
}


