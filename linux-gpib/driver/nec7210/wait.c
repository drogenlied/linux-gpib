#include "board.h"
#include <linux/sched.h>

unsigned int nec7210_wait(unsigned int status_mask)
{
	if(wait_event_interruptible(nec7210_status_wait, board.status & status_mask))
	{
		printk("gpib: wait interrupted by signal\n");
		// XXX
	}
	return board.status;
}











