

#include <board.h>


unsigned int      ibbase = IBBASE;	/* base addr of GPIB interface registers  */

uint8       ibirq  = IBIRQ;	/* interrupt request line for GPIB (1-7)  */
uint8       ibdma  = IBDMA;     /* DMA channel                            */


ibregs_t *ib = ((ibregs_t *) 0);
				/* Local pointer to IB registers */



/*
 * BDONL
 * Initialize the interface hardware.
 */
IBLCL int bdonl(int v)
{
	uint8		s;
	extern uint32	osRegAddr();

	DBGin("bdonl");

        /* ib must be set to 0 for PCIIa so we leave it untouched 
           the adress offset now is the register number :-) */

#ifndef NIPCIIa
	ib = ((struct ibregs *) osRegAddr(ibbase));
#else
	ib = (struct ibregs *) 0 ;

        switch( ibbase ){

          case 0x02e1:
          case 0x22e1:
          case 0x42e1:
          case 0x62e1:
             break;
	   default:
	     printk("PCIIa base range invalid, must be one of [0246]2e1 is %x \n",ibbase);
             return(0);
           break;
	}

        if( ibirq < 2 || ibirq > 7 ){
	  printk("Illegal Interrupt Level \n");
          return(0);
	}
#endif


	GPIBout(auxmr, AUX_CR);                     /* 7210 chip reset */
        /*GPIBout(intrt, 1);*/

	s = GPIBin(cptr);                           /* clear registers by reading */
	s = GPIBin(isr1);
	s = GPIBin(isr2);

	GPIBout(imr1, 0);                           /* disable all interrupts */
	GPIBout(imr2, 0);
	GPIBout(spmr, 0);

	GPIBout(adr,(PAD & LOMASK));                /* set GPIB address; MTA=PAD|100, MLA=PAD|040 */
#if (SAD)
	GPIBout(adr, HR_ARS | (SAD & LOMASK));      /* enable secondary addressing */
	GPIBout(admr, HR_TRM1 | HR_TRM0 | HR_ADM1);
#else
	GPIBout(adr, HR_ARS | HR_DT | HR_DL);       /* disable secondary addressing */
	GPIBout(admr, HR_TRM1 | HR_TRM0 | HR_ADM0);
#endif

	GPIBout(eosr, 0);
	GPIBout(auxmr, ICR | 5);                    /* set internal counter register N= 8 */
	GPIBout(auxmr, PPR | HR_PPU);               /* parallel poll unconfigure */
	GPIBout(auxmr, auxrabits);

	GPIBout(auxmr, AUXRB | 0);                  /* set INT pin to active high */
	GPIBout(auxmr, AUXRE | 0);

	if (v) GPIBout(auxmr, AUX_PON);	/* release pon state to bring online */

	DBGout();
	return(1);
}














