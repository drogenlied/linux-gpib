#include <board.h>
#include <asm/io.h>
#include <linux/delay.h>

/* #define REALLY_SLOW_IO */

/*
 * BDWAIT
 * Wait for a GPIB event to occur ('mask' must be non-zero).
 */

IBLCL void bdwait(unsigned int mask)

{
	uint8_t s, imr2mask;

	DBGin("bdwait");
#if USEINTS
	if (!(pgmstat & PS_NOINTS)) {
		DBGprint(DBG_BRANCH, ("use ints  "));
		udelay(2);
		s = GPIBin(ISR1);           /* clear ISRs by reading */
		udelay(2);
		s = GPIBin(ISR2);
		udelay(2);
		imr2mask = 0;
		if (mask & SRQI)
			imr2mask |= HR_SRQIE;
		if (mask & (CIC | TACS | LACS))
			imr2mask |= HR_ACIE;
		DBGprint(DBG_DATA, ("imr2mask=0x%x  ", imr2mask));
		while (!(ibstat() & mask) && NotTimedOut()) {
			ibsta = CMPL;
			udelay(2);
			GPIBout(IMR2, imr2mask);
			udelay(2);
			osWaitForInt(HR_TLCI);
			udelay(2);
			GPIBout(IMR2, 0);   /* clear IMR2 IE bits */
			udelay(2);
			s = GPIBin(ISR2);   /* clear ISR2 status bits */
			DBGprint(DBG_DATA, ("ISR2=0x%x  ", s));
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

IBLCL uint8_t bdWaitIn(void)
{
uint8_t isreg1;
DBGin("bdWaitIn");
udelay(2);
while (!((isreg1 = GPIBin(ISR1)) & HR_DI) && NotTimedOut())
  {
    udelay(2);
  }

DBGout();
return isreg1;
}

IBLCL void bdWaitOut(void)
{
DBGin("bdWaitOut");
udelay(2);
while (!(GPIBin(ISR2) & HR_CO) && NotTimedOut())
  {
    udelay(2);
  }
DBGout();
}


IBLCL void bdWaitATN(void)
{
DBGin("bdWaitATN");
while ((GPIBin(ADSR) & HR_NATN) && NotTimedOut());
DBGout();
}











