#include <ibprot.h>


/*
 * IBCAC
 * Return to the controller active state from the
 * controller standby state, i.e., turn ATN on.  Note
 * that in order to enter the controller active state
 * from the controller idle state, ibsic must be called.
 * If v is non-zero, take control synchronously, if
 * possible.  Otherwise, take control asynchronously.
 */
IBLCL int ibcac(int sync)
{
	int status = driver->update_status(driver);
	if((status & CIC) == 0)
	{
		printk("gpib: not CIC during ibcac\n");
		return -1;
	}

	driver->take_control(driver, sync);

	return 0;
}




