
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

IBLCL int ibrd(uint8_t *buf, size_t cnt)
{
	size_t requested_cnt;
	ssize_t ret;
	int status = board.update_status();

	DBGin("ibrd");
	if((status & LACS) == 0) 
	{
		ibcnt = 0;
		DBGout();
		return status;
	}
	board.go_to_standby();
	osStartTimer(timeidx);
	requested_cnt = cnt;
	// mark io in progress
	clear_bit(CMPL_NUM, &board.status);
	// initialize status to END not yet received
	clear_bit(END_NUM, &board.status);
	while ((cnt > 0) && !(board.status & (ERR | TIMO | END))) {
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
	// mark io completed
	set_bit(CMPL_NUM, &board.status);
	DBGout();
	return board.update_status();
}

