#include <ibprot.h>


/*
 * IBGTS
 * Go to the controller standby state from the controller
 * active state, i.e., turn ATN off.
 */

IBLCL int ibgts(void)
{
	int status = driver->update_status();

	if((status & CIC) == 0)
	{
		printk("gpib: not CIC during ibgts\n");
		return -1;
	}
	driver->go_to_standby();                    /* go to standby */
	return 0;
}

