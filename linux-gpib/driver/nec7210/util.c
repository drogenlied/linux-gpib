#include "board.h"

int eosmodes = 0;

/*
 * BDSRQSTAT
 * Return the 'ibsta' status of the SRQ line.
 */
IBLCL int bdSRQstat(void)
{
	int result = 0;

	DBGin("bdSRQstat");

// SRQI bit of ibsta is set by interrupt handler

	DBGout();
	return result;
}


/*
 * BDSC
 * Enable System Controller state.
 */
IBLCL void bdsc(void)
{
	DBGin("bdsc");
/*	GPIBout(cmdr, SETSC);	*/	/* set system controller */
	DBGout();
}


/* -- bdGetDataByte()
 * get last byte from bus
 */
IBLCL uint8_t bdGetDataByte(void)
{
  DBGin("bdGetDataByte");
  DBGout();
  return GPIBin(DIR);
}

/* -- bdGetCmdByte()
 * get last Cmd byte from bus
 */

IBLCL uint8_t bdGetCmdByte(void)
{
  DBGin("bdGetCmdByte");
  DBGout();
  return (GPIBin(CPTR));
}

/* -- bdGetAdrStat()
 * get address status
 */

IBLCL uint8_t bdGetAdrStat(void)
{
  DBGin("bdGetAdrStatus");
  DBGout();
  return (GPIBin(ADSR));
}


/* -- bdCheckEOI()
 * Checks if EOI is set in ADR1
 *
 */

IBLCL uint8_t bdCheckEOI(void)
{
  DBGin("bdCheckEOI");
  DBGout();
  return ( GPIBin(ADR1) & HR_EOI );
}

/* -- bdSetEOS(eos)
 * set eos byte
 *
 */

static int eosbyte = 0x0a; /*default eos byte for write operations*/


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

IBLCL uint8_t bdGetEOS(void)
{
  DBGin("bdGetEOS");
  DBGout();

  return(eosbyte);
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




