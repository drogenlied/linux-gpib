
#include <ibprot.h>


/*
 * IBRD
 * Read up to 'length' bytes of data from the GPIB into buf.  End
 * on detection of END (EOI and or EOS) and set 'end_flag'.
 *
 * NOTE:
 *      1.  The interface is placed in the controller standby
 *          state prior to beginning the read.
 *      2.  Prior to calling ibrd, the intended devices as well
 *          as the interface board itself must be addressed by
 *          calling ibcmd.
 */

IBLCL ssize_t ibrd(uint8_t *buf, size_t length, int *end_flag)
{
	size_t count = 0;
	ssize_t ret;
	int status = board.update_status();

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
	while ((count < length) && !(board.status & TIMO)) 
	{
		ret = board.read(buf, length - count, end_flag);
		if(ret < 0)
		{
			printk("gpib read error\n");
			break;
			// XXX
		}
		buf += ret;
		count += ret;
		if(*end_flag) break;
	}
	osRemoveTimer();
	// mark io completed
	set_bit(CMPL_NUM, &board.status);
	DBGout();
	return count;
}

