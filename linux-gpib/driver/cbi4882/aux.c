

#include <board.h>
#include <asm/io.h>
#include <linux/delay.h>



/* -- bdSendAuxCmd(int cmd)
 * Performs single auxiliary commands (nec7210)
 *
 *
 */

IBLCL void bdSendAuxCmd(int cmd)
{
  DBGin("bdSendAuxCmd");
  switch(cmd){

  case AUX_GTS:
    DBGprint(DBG_BRANCH,("Aux Send GTS"));
    GPIBout(AUXMR,AUX_GTS); 
    break;
  case AUX_TCS:
    DBGprint(DBG_BRANCH,("Aux Send TCS"));

    /* Huh, the CBI4882 chip seems to have a serious bug 
       that need to send TCS or TCA twice, otherwise an adress change 
       leaves ATN asserted */

    GPIBout(AUXMR,AUX_TCS);
#if 1
    udelay(2);
    GPIBout(AUXMR,AUX_TCS);
#endif
    break;
  case AUX_TCA:
    DBGprint(DBG_BRANCH,("Aux Send TCA"));

    GPIBout(AUXMR,AUX_TCA);
#if 1
    udelay(2);
    GPIBout(AUXMR,AUX_TCA);
#endif
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
#if 1
    udelay(2);
    GPIBout(AUXMR,AUX_FH);
#endif
    break;
  case AUX_SEOI:
    if( pgmstat & PS_NOEOI){ 
      DBGprint(DBG_BRANCH,("Aux Send SEOI disabled"));
    } else {
      DBGprint(DBG_BRANCH,("Aux Send EOI"));
      GPIBout(AUXMR, AUX_SEOI);
    }
    break;  
  default:
    DBGprint(DBG_BRANCH,(" warning: illegal auxiliary command"));
    GPIBout(AUXMR,cmd);
    break;
  }
  bdlines();
  DBGout();
}



/* -- bdSendAuxACmd(int cmd)
 * Set Auxiliary A Register
 *
 *
 */

IBLCL void bdSendAuxACmd(int cmd)
{
  DBGin("bdSendAuxACmd");
  DBGprint(DBG_BRANCH,("Aux Send=0x%x",cmd));

  GPIBout(AUXMR, auxrabits | cmd);

  DBGout();
}



