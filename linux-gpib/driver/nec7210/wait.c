#include "board.h"
#include <linux/sched.h>

IBLCL void bdwait(unsigned int mask)
{
	DBGin("bdwait");

	ibsta = CMPL;
	// XXX check for failure due to signal
	wait_event_interruptible(nec7210_status_wait, ibsta & mask);

	DBGout();
}

/***********************************************************************/


/* -- int bdWaitIn(void)
 * Wait for Input
 *
 */

IBLCL int bdWaitIn(void)
{
uint8 isreg1;
DBGin("bdWaitIn");

    /* polling */
//    while (!((isreg1 = GPIBin(ISR1)) & HR_DI) && !(isreg1 & HR_END ) && NotTimedOut());
    /*DBGprint(DBG_DATA, ("***ISR1 =0x%x", isreg1 ));*/
DBGout();
//    if ( (isreg1 & HR_END) || TimedOut() )
//      return (-1);
//    else
      return -1;
}


/***********************************************************************/

/* -- bdWaitOut(void)
 * Wait for Output
 *
 */

IBLCL void bdWaitOut(void)
{
DBGin("bdWaitOut");
	wait_event_interruptible(nec7210_write_wait, test_bit(0, &command_out_ready));
	clear_bit(0, &command_out_ready);
DBGout();
}

/* -- bdWaitATN(void)
 * Wait for *ATN
 *
 */

IBLCL void bdWaitATN(void)
{
DBGin("bdWaitATN");
while ((GPIBin(ADSR) & HR_NATN) && NotTimedOut());
DBGout();
}











