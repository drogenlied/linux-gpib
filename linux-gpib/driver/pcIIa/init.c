

#include <board.h>


unsigned long ibbase = IBBASE;	/* base addr of GPIB interface registers  */

uint8       ibirq  = IBIRQ;	/* interrupt request line for GPIB (1-7)  */
uint8       ibdma  = IBDMA;     /* DMA channel                            */

/*
 * BDONL
 * Initialize the interface hardware.
 */
IBLCL int bdonl(int v)
{
	uint8_t s;

	DBGin("bdonl");

#ifdef NIPCIIa
	switch( ibbase ){

		case 0x02e1:
		case 0x22e1:
		case 0x42e1:
		case 0x62e1:
			break;
	   default:
	     printk("PCIIa base range invalid, must be one of [0246]2e1 is %lx \n", ibbase);
             return(0);
           break;
	}

        if( ibirq < 2 || ibirq > 7 ){
	  printk("Illegal Interrupt Level \n");
          return(0);
	}
#endif


	GPIBout(AUXMR, AUX_CR);                     /* 7210 chip reset */
        /*GPIBout(intrt, 1);*/

	s = GPIBin(CPTR);                           /* clear registers by reading */
	s = GPIBin(ISR1);
	s = GPIBin(ISR2);

	GPIBout(IMR1, 0);                           /* disable all interrupts */
	GPIBout(IMR2, 0);
	GPIBout(SPMR, 0);

	GPIBout(ADR,(PAD & LOMASK));                /* set GPIB address; MTA=PAD|100, MLA=PAD|040 */
#if (SAD)
	GPIBout(ADR, HR_ARS | (SAD & LOMASK));      /* enable secondary addressing */
	GPIBout(ADMR, HR_TRM1 | HR_TRM0 | HR_ADM1);
#else
	GPIBout(ADR, HR_ARS | HR_DT | HR_DL);       /* disable secondary addressing */
	GPIBout(ADMR, HR_TRM1 | HR_TRM0 | HR_ADM0);
#endif

	GPIBout(EOSR, 0);
	GPIBout(AUXMR, ICR | 5);                    /* set internal counter register N= 8 */
	GPIBout(AUXMR, PPR | HR_PPU);               /* parallel poll unconfigure */
	GPIBout(AUXMR, auxrabits);

	GPIBout(AUXMR, AUXRB | 0);                  /* set INT pin to active high */
	GPIBout(AUXMR, AUXRE | 0);

	if (v) GPIBout(AUXMR, AUX_PON);	/* release pon state to bring online */

	DBGout();
	return(1);
}














