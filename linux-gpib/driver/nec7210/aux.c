

#include <board.h>





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



