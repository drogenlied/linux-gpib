#include "board.h"
#include <linux/sched.h>

unsigned int nec7210_wait(unsigned int status_mask)
{
	ibsta = CMPL;	// XXX
	if(wait_event_interruptible(nec7210_status_wait, ibsta & status_mask))
	{
		printk("gpib: wait interrupted by signal\n");
		// XXX
	}
	return ibsta;
}











