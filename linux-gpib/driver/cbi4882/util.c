
#include <board.h>



int eosmodes = 0;


/*
 * BDSRQSTAT
 * Return the 'ibsta' status of the SRQ line.
 */
IBLCL int bdSRQstat(void)
{
	int	result;

	DBGin("bdSRQstat");
#if 0
	result = (GPIBin(isr3) & HR_SRQI_CIC) ? SRQI : 0;
#endif
/*@@*/
	result = (GPIBin(isr2) & HR_SRQI) ? SRQI : 0;
        /*result=0;*/       
         /* quick and dirty hack */
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
  return GPIBin(dir);
}

/* -- bdGetCmdByte()
 * get last Cmd byte from bus
 */

IBLCL uint8 bdGetCmdByte(void)
{
  DBGin("bdGetCmdByte");
  DBGout();
  return (GPIBin(cptr));
}

/* -- bdGetAdrStat()
 * get address status
 */

IBLCL uint8 bdGetAdrStat(void)
{
  DBGin("bdGetAdrStatus");
  DBGprint( DBG_DATA, ("--adrstat=0x%x",GPIBin(adsr)));
  DBGout();
  return (GPIBin(adsr));
}


/* -- bdCheckEOI()
 * Checks if EOI is set in adr1
 *
 */

IBLCL uint8 bdCheckEOI(void)
{
  DBGin("bdCheckEOI");
  DBGout();
  return ( GPIBin(adr1) & HR_EOI );
}

/* -- bdSetEOS(eos)
 * set eos byte
 *
 */

static int eosbyte = 0x0a; /*default eos byte for write operations*/


IBLCL void bdSetEOS(int ebyte)
{
  DBGin("bdSetEOS");
  GPIBout(eosr, ebyte);
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
	GPIBout(spmr, 0);		/* clear current serial poll status */
	GPIBout(spmr, v);		/* set new status to v */
  DBGout();
}


/* -- bdSetPAD(reg)
 * Sets PAD of Controller
 *
 */

IBLCL void bdSetPAD(int v)
{
  DBGin("bdSetPAD");
  GPIBout(adr,( v & LOMASK ));
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
    GPIBout(adr, HR_ARS | (mySAD & LOMASK));
    GPIBout(admr, HR_TRM1 | HR_TRM0 | HR_ADM1);
  } else {
    GPIBout(adr, HR_ARS | HR_DT | HR_DL);
    GPIBout(admr, HR_TRM1 | HR_TRM0 | HR_ADM0);
  }
  DBGout();
}




