
#include "board.h"

/* @BEGIN-MAN

\routine{  bdonl(v)  }
\synopsis{  }
\description{ Initialize the interface hardware. }
\returns{   }
\bugs{   }

   @END-MAN */

unsigned long ibbase = IBBASE;
uint8 ibirq = IBIRQ;
uint8 ibdma = IBDMA;

/*
 * BDONL
 * Initialize the interface hardware.
 */
IBLCL int bdonl(int v)
{
  uint8		s;
  int		i;

  DBGin("bdonl");

  if(v){
    s = GPIBpgin(CSR) >> 4;	/* read chip-signature */
    if( s != 0x2 ){
      if( s != 0x3 ) { /* not a NAT488BPL or TNT4882 */
	if (s==0) { 
	  /* Check for old NI card with Turbo 488 & NEC 7210 
             from jlavi@cs.joensuu.fi (Jarkko Lavinen) */

	  /* Check if FIFO accepts bytes */
	  GPIBout(CFG,0); 
	  GPIBout(CMDR,RSTFIFO);
	  GPIBout(CNTL, -16);
	  GPIBout(CNTH, -16 >> 8);
	  s = GPIBin(ISR3);
	  if (s & HR_NEF || !(s & HR_NFF)){
            DBGout(); return 0;}		
            /* FIFO not empty or full or not NI board*/
	  for(i=0;i<20;i++) {
	    GPIBout(FIFOB, (i+42)^(i-488));
	    if (! ((s=GPIBin(ISR3)) & HR_NFF)) break;
	  }
	  /* FIFO should be full exactly after 16 byte writes */
	  /* status is 0x15 at this point on my card */
	  if (i!=15) { DBGout(); return 0; }

	  /* Now read bytes back from FIFO */
	  GPIBout(CNTL,-16);
	  GPIBout(CNTH,-16 >> 8);
	  GPIBout(CFG,C_IN|C_CCEN);
	  GPIBout(CMDR,GO);

	  for(i=0;i<20;i++) {
	    if ( (uint8)((i+42)^(i-488)) != GPIBin(FIFOB)){
	      DBGout(); return 0;		
              /* Fifo not working or not NI board */
	    }
	    if (!(s=GPIBin(ISR3) & HR_NEF)) break;
	  }
	  /* FIFO should be empty exactly after 16 byte reads and
	     also full due to roundoff */

	  if (i != 15 || s!=0) {DBGout();return 0;}
	  /* Ok */
	} else {DBGout();return 0;}
      }
    }
  }


  GPIBout(CMDR, SFTRST);	/* Turbo488 software reset */
  GPIBout(CMDR, CLRSC);		/* by default, disable system controller */
  GPIBout(AUXMR, AUX_CR);	/* 7210 chip reset */
  GPIBout(INTRT, 1);

  s = GPIBin(CPTR);		/* clear registers by reading */
  s = GPIBin(ISR1);
  s = GPIBin(ISR2);

  GPIBout(IMR1, 0);		/* disable all interrupts */
  GPIBout(IMR2, 0);
  GPIBout(SPMR, 0);
  GPIBout(ADR,(PAD & LOMASK));	/* set GPIB address; MTA=PAD|100, MLA=PAD|040 */
#if (SAD)
  GPIBout(ADR, HR_ARS | (SAD & LOMASK)); /* enable secondary addressing */
  GPIBout(ADMR, HR_TRM1 | HR_TRM0 | HR_ADM1);
#else
  GPIBout(ADR, HR_ARS | HR_DT | HR_DL);	/* disable secondary addressing */
  GPIBout(ADMR, HR_TRM1 | HR_TRM0 | HR_ADM0);
#endif
  GPIBout(EOSR, 0);
  GPIBout(AUXMR, ICR | 8);	/* set internal counter register N= 8 */
  GPIBout(AUXMR, PPR | HR_PPU);	/* parallel poll unconfigure */
  GPIBout(AUXMR, auxrabits);
  GPIBout(AUXMR, AUXRB | 0);	/* set INT pin to active high */
  GPIBout(AUXMR, AUXRB | HR_TRI);
  GPIBout(AUXMR, AUXRE | 0);
  GPIBout(TIMER, 0xC4);		/* 0xC4 = 7.5 usec (60 * 0.125) */

  if (v) GPIBout(AUXMR, AUX_PON); /* release pon state to bring online */

  DBGout();
  return(1);
}

