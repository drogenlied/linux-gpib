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

	// XXX
	
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
IBLCL uint8 bdGetDataByte(void)
{
  DBGin("bdGetDataByte");
  DBGout();
  return GPIBin(DIR);
}

/* -- bdGetCmdByte()
 * get last Cmd byte from bus
 */

IBLCL uint8 bdGetCmdByte(void)
{
  DBGin("bdGetCmdByte");
  DBGout();
  return (GPIBin(CPTR));
}

/* -- bdGetAdrStat()
 * get address status
 */

IBLCL uint8 bdGetAdrStat(void)
{
  DBGin("bdGetAdrStatus");
  DBGout();
  return (GPIBin(ADSR));
}


/* -- bdCheckEOI()
 * Checks if EOI is set in ADR1
 *
 */

IBLCL uint8 bdCheckEOI(void)
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


IBLCL void bdSetEOS(int ebyte)
{
  DBGin("bdSetEOS");
  GPIBout(EOSR, ebyte);
  eosbyte = ebyte;
  DBGout();
}

IBLCL uint8 bdGetEOS(void)
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




