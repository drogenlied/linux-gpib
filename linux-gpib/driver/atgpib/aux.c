

#include <board.h>


/* @BEGIN-MAN

\routine{  bdSendAuxCmd(int cmd)  }
%\synopsis{  }
\description{ Performs Single Wire Messages. With nec7210 this is implemented
with the Auxiliary Register commands. }
%\returns{   }
%\bugs{   }

   @END-MAN */



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
    GPIBout(AUXMR,AUX_GTS);
    break;
  case AUX_TCS:
    GPIBout(AUXMR,AUX_TCS);
    break;
  case AUX_TCA:
    GPIBout(AUXMR,AUX_TCA);
    break;
  case AUX_EPP:
    GPIBout(AUXMR,AUX_EPP);
    break;
  case AUX_SIFC:
    GPIBout(AUXMR,AUX_SIFC);
    break;
  case AUX_SREN:
    GPIBout(AUXMR,AUX_SREN);
    break;
  case AUX_CREN:
    GPIBout(AUXMR,AUX_CREN);
    break;
  case AUX_CIFC:
    GPIBout(AUXMR,AUX_CIFC);
    break;
  case AUX_FH:
    GPIBout(AUXMR,AUX_FH);
    break;
       
  default:
    DBGprint(DBG_BRANCH,(" warning: illegal auxiliary command"));
    GPIBout(AUXMR,cmd);
    break;
  }
#if DEBUG
bdlines();
#endif
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
  GPIBout(AUXMR, auxrabits | cmd);
  DBGout();
}
