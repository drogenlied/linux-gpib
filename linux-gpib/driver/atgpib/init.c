

#include <board.h>


unsigned int      ibbase = IBBASE;	/* base addr of GPIB interface registers  */
uint8       ibirq  = IBIRQ;	/* interrupt request line for GPIB (1-7)  */
uint8       ibdma  = IBDMA;     /* DMA channel                            */


ibregs_t *ib = ((ibregs_t *) 0);
				/* Local pointer to IB registers */

/* @BEGIN-MAN

\routine{  bdonl(v)  }
\synopsis{  }
\description{ Initialize the interface hardware. }
\returns{   }
\bugs{   }

   @END-MAN */


/*
 * BDONL
 * Initialize the interface hardware.
 */
IBLCL int bdonl(int v)
{
  uint8		s;
  int		i;           
  extern uint32	osRegAddr();

  DBGin("bdonl");
  ib = (struct ibregs *) osRegAddr(ibbase);

  if(v){
    s = GPIBpgin(csr) >> 4;	/* read chip-signature */
    if( s != 0x2 ){
      if( s != 0x3 ) { /* not a NAT488BPL or TNT4882 */
	if (s==0) { 
	  /* Check for old NI card with Turbo 488 & NEC 7210 
             from jlavi@cs.joensuu.fi (Jarkko Lavinen) */

	  /* Check if FIFO accepts bytes */
	  GPIBout(cfg,0); 
	  GPIBout(cmdr,RSTFIFO);
	  GPIBout(cntl, -16);
	  GPIBout(cnth, -16 >> 8);
	  s = GPIBin(isr3);
	  if (s & HR_NEF || !(s & HR_NFF)){
            DBGout(); return 0;}		
            /* FIFO not empty or full or not NI board*/
	  for(i=0;i<20;i++) {
	    GPIBout(fifo.f8.b,(i+42)^(i-488));
	    if (! ((s=GPIBin(isr3)) & HR_NFF)) break;
	  }
	  /* FIFO should be full exactly after 16 byte writes */
	  /* status is 0x15 at this point on my card */
	  if (i!=15) { DBGout(); return 0; }

	  /* Now read bytes back from FIFO */
	  GPIBout(cntl,-16);
	  GPIBout(cnth,-16 >> 8);
	  GPIBout(cfg,C_IN|C_CCEN);
	  GPIBout(cmdr,GO);

	  for(i=0;i<20;i++) {
	    if ( (uint8)((i+42)^(i-488)) != GPIBin(fifo.f8.b)){
	      DBGout(); return 0;		
              /* Fifo not working or not NI board */
	    }
	    if (!(s=GPIBin(isr3) & HR_NEF)) break;
	  }
	  /* FIFO should be empty exactly after 16 byte reads and
	     also full due to roundoff */

	  if (i != 15 || s!=0) {DBGout();return 0;}
	  /* Ok */
	} else {DBGout();return 0;}
      }
    }
  }


  GPIBout(cmdr, SFTRST);	/* Turbo488 software reset */
  GPIBout(cmdr, CLRSC);		/* by default, disable system controller */
  GPIBout(auxmr, AUX_CR);	/* 7210 chip reset */
  GPIBout(intrt, 1);

  s = GPIBin(cptr);		/* clear registers by reading */
  s = GPIBin(isr1);
  s = GPIBin(isr2);

  GPIBout(imr1, 0);		/* disable all interrupts */
  GPIBout(imr2, 0);
  GPIBout(spmr, 0);
  GPIBout(adr,(PAD & LOMASK));	/* set GPIB address; MTA=PAD|100, MLA=PAD|040 */
#if (SAD)
  GPIBout(adr, HR_ARS | (SAD & LOMASK)); /* enable secondary addressing */
  GPIBout(admr, HR_TRM1 | HR_TRM0 | HR_ADM1);
#else
  GPIBout(adr, HR_ARS | HR_DT | HR_DL);	/* disable secondary addressing */
  GPIBout(admr, HR_TRM1 | HR_TRM0 | HR_ADM0);
#endif
  GPIBout(eosr, 0);
  GPIBout(auxmr, ICR | 8);	/* set internal counter register N= 8 */
  GPIBout(auxmr, PPR | HR_PPU);	/* parallel poll unconfigure */
  GPIBout(auxmr, auxrabits);
  GPIBout(auxmr, AUXRB | 0);	/* set INT pin to active high */
  GPIBout(auxmr, AUXRB | HR_TRI);
  GPIBout(auxmr, AUXRE | 0);
  GPIBout(timer, 0xC4);		/* 0xC4 = 7.5 usec (60 * 0.125) */

  if (v) GPIBout(auxmr, AUX_PON); /* release pon state to bring online */

  DBGout();
  return(1);
}

