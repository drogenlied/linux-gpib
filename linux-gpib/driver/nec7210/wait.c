#include "board.h"
#include <linux/sched.h>

/*
 * BDWAIT
 * Wait for a GPIB event to occur ('mask' must be non-zero).
 */
IBLCL void bdwait(unsigned int mask)
{
	uint8_t imr2mask = 0;

	DBGin("bdwait");

#if USEINTS
	if (!(pgmstat & PS_NOINTS))
	{
		DBGprint(DBG_BRANCH, ("use ints  "));

		imr2mask = 0;
		if (mask & SRQI)
			imr2mask |= HR_SRQIE;
		if (mask & (CIC | TACS | LACS))
			imr2mask |= HR_ACIE;
		DBGprint(DBG_DATA, ("imr2mask=0x%x  ", imr2mask));

		ibsta = CMPL;
		printk("sleep on\n");
		// XXX test for failure
		interruptible_sleep_on(&nec7210_wait);
		printk("woke\n");
	}
	else
#endif
		ibsta = CMPL;

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
    while (!((isreg1 = GPIBin(ISR1)) & HR_DI) && !(isreg1 & HR_END ) && NotTimedOut());
    /*DBGprint(DBG_DATA, ("***ISR1 =0x%x", isreg1 ));*/
DBGout();
    if ( (isreg1 & HR_END) || TimedOut() ) 
      return (-1);
    else 
      return (isreg1);
}


/***********************************************************************/

/* -- bdWaitOut(void)
 * Wait for Output 
 *
 */

IBLCL void bdWaitOut(void)
{
DBGin("bdWaitOut");
while (!((GPIBin(ISR2) & HR_CO) || (GPIBin(ISR1) & HR_DO) ) && NotTimedOut());
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











