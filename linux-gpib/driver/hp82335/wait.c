#include <board.h>

extern uint8 ibirq;

/*
 * BDWAIT
 * Wait for a GPIB event to occur ('mask' must be non-zero).
 */
IBLCL void bdwait(unsigned int mask)
{
	uint8 s1, s2 = 0, imr2mask = 0;

	DBGin("bdwait");

#if USEINTS
	if (!(pgmstat & PS_NOINTS)) {
		DBGprint(DBG_BRANCH, ("use ints  "));
		s1 = GPIBin(isr0)  ;           /* clear ISRs by reading */
		s2 = GPIBin(isr1)  ;

   		GPIBout(imr1, 0);   /* clear imr1 IE bits */

		imr2mask = 0;
		if (mask & SRQI)
			imr2mask |= HR_SRQIE;
		if (mask & (CIC | TACS | LACS))
			imr2mask |= HR_MACIE;
		DBGprint(DBG_DATA, ("imr2mask=0x%x  ", imr2mask));

		while (!(s2 & imr2mask) && NotTimedOut()) {
                          /* was ibstat() & mask */
			ibsta = CMPL;
			osWaitForInt(imr2mask); 
                           /* the imr3mask is imr2 for PCII */
			s2 = GPIBin(isr1);
			DBGprint(DBG_DATA, ("***isr1=0x%x ", s2));
			s1 = GPIBin(isr0);   /* clear isr2 status bits */
			DBGprint(DBG_DATA, ("***isr0=0x%x ", s1));
   			GPIBout(imr0, 0);   /* clear imr1 IE bits */
   			GPIBout(imr1, 0);   /* clear imr2 IE bits */

		}
	}
	else
#endif

waitdone:

	while (!(s2 & imr2mask) && NotTimedOut())
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
uint8 isreg1,s;
DBGin("bdWaitIn");

    /* polling */
    while (!((isreg1 = GPIBin(isr0)) & HR_BI) && !(isreg1 & HR_END ) && NotTimedOut());

    /*DBGprint(DBG_DATA, ("***isr0 =0x%x", isreg1 ));*/
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
while (!((GPIBin(isr0) & HR_BO))  && NotTimedOut());
DBGout();
}

/* -- bdWaitATN(void)
 * Wait for *ATN
 *
 */

IBLCL void bdWaitATN(void)
{
DBGin("bdWaitATN");
while (!((GPIBin(adsr) & HR_ATN)) && NotTimedOut());
DBGout();
}











