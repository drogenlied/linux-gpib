

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
    GPIBout(auxmr,AUX_GTS);
    break;
  case AUX_TCS:
    GPIBout(auxmr,AUX_TCS);
    break;
  case AUX_TCA:
    GPIBout(auxmr,AUX_TCA);
    break;
  case AUX_EPP:
    GPIBout(auxmr,AUX_EPP);
    break;
  case AUX_SIFC:
    GPIBout(auxmr,AUX_SIFC);
    break;
  case AUX_SREN:
    GPIBout(auxmr,AUX_SREN);
    break;
  case AUX_CREN:
    GPIBout(auxmr,AUX_CREN);
    break;
  case AUX_CIFC:
    GPIBout(auxmr,AUX_CIFC);
    break;
  case AUX_FH:
    GPIBout(auxmr,AUX_FH);
    break;
       
  default:
    DBGprint(DBG_BRANCH,(" warning: illegal auxiliary command"));
    GPIBout(auxmr,cmd);
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
  GPIBout(auxmr, auxrabits | cmd);
  DBGout();
}
