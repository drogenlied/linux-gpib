#include <board.h>
#include <asm/io.h>

/* #define REALLY_SLOW_IO */

/*
 * BDWAIT
 * Wait for a GPIB event to occur ('mask' must be non-zero).
 */

IBLCL void bdwait(unsigned int mask)

{
	uint8 s, imr2mask;

	DBGin("bdwait");
#if USEINTS
	if (!(pgmstat & PS_NOINTS)) {
		DBGprint(DBG_BRANCH, ("use ints  "));
		SLOW_DOWN_IO;
		s = GPIBin(isr1);           /* clear ISRs by reading */
		SLOW_DOWN_IO;
		s = GPIBin(isr2);
		SLOW_DOWN_IO;
		imr2mask = 0;
		if (mask & SRQI)
			imr2mask |= HR_SRQIE;
		if (mask & (CIC | TACS | LACS))
			imr2mask |= HR_ACIE;
		DBGprint(DBG_DATA, ("imr2mask=0x%x  ", imr2mask));
		while (!(ibstat() & mask) && NotTimedOut()) {
			ibsta = CMPL;
			SLOW_DOWN_IO;
			GPIBout(imr2, imr2mask);
			SLOW_DOWN_IO;
			osWaitForInt(HR_TLCI);
			SLOW_DOWN_IO;
			GPIBout(imr2, 0);   /* clear imr2 IE bits */
			SLOW_DOWN_IO;
			s = GPIBin(isr2);   /* clear isr2 status bits */
			DBGprint(DBG_DATA, ("isr2=0x%x  ", s));
		}
	}
	else
#endif
	while (!(ibstat() & mask) && NotTimedOut())
		ibsta = CMPL;
	DBGout();
}

/***********************************************************************/

/* -- int bdWaitIn(void)
 * Wait for Input
 *
 */

IBLCL uint8 bdWaitIn(void)
{
uint8 isreg1;
DBGin("bdWaitIn");
SLOW_DOWN_IO;
while (!((isreg1 = GPIBin(isr1)) & HR_DI) && NotTimedOut())
  {
    SLOW_DOWN_IO;
  }

DBGout();
return isreg1;
}

IBLCL void bdWaitOut(void)
{
DBGin("bdWaitOut");
SLOW_DOWN_IO;
while (!(GPIBin(isr2) & HR_CO) && NotTimedOut())
  {
    SLOW_DOWN_IO;
  }
DBGout();
}


IBLCL void bdWaitATN(void)
{
DBGin("bdWaitATN");
while ((GPIBin(adsr) & HR_NATN) && NotTimedOut());
DBGout();
}











