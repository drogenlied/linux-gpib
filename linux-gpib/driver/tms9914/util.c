
#include <board.h>

/*
 * BDSRQSTAT
 * Return the 'ibsta' status of the SRQ line.
 */
IBLCL int bdSRQstat(void)
{
	int	result = 0;

	DBGin("bdSRQstat");
#if 0
	result = (GPIBin(ISR3) & HR_SRQI_CIC) ? SRQI : 0;
//XXX
	result = (GPIBin(ISR2) & HR_SRQI) ? SRQI : 0;
        /*result=0;*/       
         /* quick and dirty hack */
#endif
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

  uint8_t stat;

  DBGin("bdGetAdrStatus");

  stat = GPIBin(ADSR);
  DBGprint(DBG_DATA,("ADSR=0x%x",stat));
  DBGout();
  return stat;
}


/* -- bdCheckEOI()
 * Checks if EOI is set in ADR1
 *
 */

IBLCL uint8_t bdCheckEOI(void)
{
  DBGin("bdCheckEOI not implemented");
  DBGout();
#if 0
  return ( GPIBin(ADR1) & HR_EOI );
#endif
	return 0;
}

/* -- bdSetEOS(eos)
 * set eos byte
 *
 */

static int eosbyte = 0x0a; /*default eos byte for write operations*/
int eosmodes = 0;


IBLCL void bdSetEOS(int ebyte)
{
  DBGin("bdSetEOS ");

  eosbyte = ebyte;
  DBGout();
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
  DBGin("bdSetSAD");
  if(enable){
    DBGprint(DBG_DATA, ("sad=0x%x  ", mySAD));
    GPIBout(ADR, HR_EDPA | (mySAD & LOMASK));
    /*GPIBout(ADMR, HR_TRM1 | HR_TRM0 | HR_ADM1);*/
  } else {
    GPIBout(ADR, HR_EDPA | HR_DAT | HR_DAL);
    /*GPIBout(ADMR, HR_TRM1 | HR_TRM0 | HR_ADM0);*/
  }
  DBGout();
}




