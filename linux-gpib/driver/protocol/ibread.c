
#include <ibprot.h>


/*
 * IBRD
 * Read up to 'length' bytes of data from the GPIB into buf.  End
 * on detection of EOI.
 *
 * NOTE:
 *      1.  The interface is placed in the controller standby
 *          state prior to beginning the read.
 *      2.  Prior to calling ibrd, the intended devices as well
 *          as the interface board itself must be addressed by
 *          calling ibcmd.
 */

IBLCL ssize_t ibrd(uint8_t *buf, size_t length)
{
	size_t count = 0;
	ssize_t ret;
	int status = board.update_status();
printk("entering ibrd\n");

	DBGin("ibrd");
	if((status & LACS) == 0) 
	{
		printk("gpib read failed: not listener\n");
		DBGout();
		return -1;
	}
	board.go_to_standby();
	osStartTimer(timeidx);
	// mark io in progress
	clear_bit(CMPL_NUM, &board.status);
	// initialize status to END not yet received
	clear_bit(END_NUM, &board.status);
	while ((count < length) && !(board.status & (ERR | TIMO | END))) {
		ret = board.read(buf, length - count, 0);	// eos XXX
		if(ret < 0)
		{
			printk("gpib read error\n");
			break;
			// XXX
		}
		buf += ret;
		count += ret;
printk("board status 0x%x\n", board.status);
	}
	osRemoveTimer();
	// mark io completed
	set_bit(CMPL_NUM, &board.status);
	DBGout();
	return count;
}

