#include "board.h"

void nec7210_enable_eos(uint8_t eos_byte, int compare_8_bits)
{
	DBGin("bdSetEOS");
	GPIBout(EOSR, eos_byte);
	auxa_bits |= HR_REOS;
	if(compare_8_bits)
		auxa_bits |= HR_BIN;
	else
		auxa_bits &= ~HR_BIN;
	GPIBout(AUXMR, auxa_bits);
	DBGout();
}

void nec7210_disable_eos(void)
{
	auxa_bits &= ~HR_REOS;
	GPIBout(AUXMR, auxa_bits);
}

/* -- bdSetSPMode(reg)
 * Sets Serial Poll Mode
 *
 */

IBLCL void bdSetSPMode(int v)
{
  DBGin("bdSetSPMode");
	GPIBout(SPMR, 0);		/* clear current serial poll status */
	GPIBout(SPMR, v);		/* set new status to v */
  DBGout();
}


/* -- bdSetPAD(reg)
 * Sets PAD of Controller
 *
 */

IBLCL void bdSetPAD(int v)
{
  DBGin("bdSetPAD");
  GPIBout(ADR,( v & LOMASK ));
  DBGout();
}

/* -- bdSetSAD()
 * Sets SAD of Controller
 *
 */

IBLCL void bdSetSAD(int mySAD,int enable)
{
  DBGin("bdSetSPMode");
  if(enable){
    DBGprint(DBG_DATA, ("sad=0x%x  ", mySAD));
    GPIBout(ADR, HR_ARS | (mySAD & LOMASK));
    GPIBout(ADMR, HR_TRM1 | HR_TRM0 | HR_ADM1);
  } else {
    GPIBout(ADR, HR_ARS | HR_DT | HR_DL);
    GPIBout(ADMR, HR_TRM1 | HR_TRM0 | HR_ADM0);
  }
  DBGout();
}

unsigned int nec7210_update_status(void)
{
	int address_status_bits = GPIBin(ADSR);

	/* everything but ATN is updated by
	 * interrupt handler */
	if(address_status_bits & HR_NATN)
		clear_bit(ATN_NUM, &board.status);
	else
		set_bit(ATN_NUM, &board.status);

	return board.status;
}


