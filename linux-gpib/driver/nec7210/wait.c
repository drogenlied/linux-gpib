#include "board.h"
#include <linux/sched.h>

IBLCL void bdwait(unsigned int mask)
{
	DBGin("bdwait");

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











