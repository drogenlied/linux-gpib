
#include "board.h"
#include <linux/wait.h>
#include <asm/bitops.h>

/* -- bdSendAuxCmd(int cmd)
 * Performs single auxiliary commands (nec7210)
 *
 *
 */

void bdSendAuxCmd(int cmd)
{
  DBGin("bdSendAuxCmd");
  switch(cmd){

  case AUX_GTS:
    DBGprint(DBG_BRANCH,("Aux Send GTS"));
    GPIBout(AUXMR,AUX_GTS); 
    break;
  case AUX_TCS:
    DBGprint(DBG_BRANCH,("Aux Send TCS"));
    GPIBout(AUXMR,AUX_TCS);
    break;
  case AUX_TCA:
    DBGprint(DBG_BRANCH,("Aux Send TCA"));
    GPIBout(AUXMR,AUX_TCA);
    break;
  case AUX_EPP:
    DBGprint(DBG_BRANCH,("Aux Send EPP"));
    GPIBout(AUXMR,AUX_EPP);
    break;
  case AUX_SIFC:
    DBGprint(DBG_BRANCH,("Aux Send IFC"));
    GPIBout(AUXMR,AUX_SIFC); 
    break;
  case AUX_SREN:
    DBGprint(DBG_BRANCH,("Aux Send REM"));
    GPIBout(AUXMR,AUX_SREN);
    break;
  case AUX_CREN:
    DBGprint(DBG_BRANCH,("Aux Send unset REM"));
    GPIBout(AUXMR,AUX_CREN);
    break;
  case AUX_CIFC:
    DBGprint(DBG_BRANCH,("Aux Send unset IFC"));
    GPIBout(AUXMR,AUX_CIFC);
    break;
  case AUX_FH:
    DBGprint(DBG_BRANCH,("Aux Send FH"));
    GPIBout(AUXMR,AUX_FH);
    break;
  case AUX_SEOI:
    DBGprint(DBG_BRANCH,("Aux Send EOI"));
    GPIBout(AUXMR, AUX_SEOI);
    break;
  default:
    DBGprint(DBG_BRANCH,(" warning: illegal auxiliary command"));
    GPIBout(AUXMR,cmd);
    break;
  }
  bdlines();
  DBGout();
}

int nec7210_take_control(int syncronous)
{
	if(syncronous)
		GPIBout(AUXMR, AUX_TCS);
	else
		GPIBout(AUXMR, AUX_TCA);
	// wait until we have control
	while(GPIBin(ADSR) & HR_NATN)
	{
		printk("waiting on ATN\n");
		if(interruptible_sleep_on_timeout(&nec7210_status_wait, 100))
		{
			printk("error waiting for ATN\n");
			return -1;
		}
	}
	return 0;
}

int nec7210_go_to_standby(void)
{
	GPIBout(AUXMR, AUX_GTS);
	while((GPIBin(ADSR) & HR_NATN) == 0)
	{
		printk("waiting on NATN\n");
		if(interruptible_sleep_on_timeout(&nec7210_status_wait, 100))
		{
			printk("error waiting for NATN\n");
			return -1;
		}
	}
	return 0;
}

void nec7210_interface_clear(int assert)
{
	if(assert)
		GPIBout(AUXMR, AUX_SIFC);
	else
		GPIBout(AUXMR, AUX_CIFC);
}

void nec7210_remote_enable(int enable)
{
	if(enable)
		GPIBout(AUXMR, AUX_SREN);
	else
		GPIBout(AUXMR, AUX_CREN);
}
