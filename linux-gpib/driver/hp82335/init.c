#include "board.h"

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

	DBGin("bdonl");

        /*printk("base=0x%lx\n",ibbase);*/

#if defined(ZIATECH)
	printk("Ziatech: set base to 0x%lx \n ",ibbase);
	printk("Ziatech: ISR1 = 0x%lx \n ",ibbase + ISR1);
	printk("Ziatech: adswr= 0x%lx \n ",ibbase + ADSWR);
#endif
#if defined(HP82335)
        switch( ibbase ){

          case 0xC000:
          case 0xC400:
          case 0xC800:
          case 0xCC00:
          case 0xD000:
          case 0xD400:
          case 0xD800:
          case 0xDC00:
          case 0xE000:
          case 0xE400:
          case 0xE800:
          case 0xEC00:
          case 0xF000:
          case 0xF400:
          case 0xF800:
          case 0xFC00:

             break;
	   default:
	     printk("hp82335 base range 0x%lx invalid, see Hardware Manual\n",ibbase);
             DBGout(); return(0);
           break;
	}


        if( ibirq < 3 || ibirq > 7 ){
	  printk("Illegal Interrupt Level must be within 3..7\n");
          DBGout(); return(0);
	}


	ibbase = (unsigned long) ioremap(ibbase, 0x4000);          /* setting base address */
        /*printk("io remap=0x%x\n",ibbase);*/

#endif

	GPIBout(AUXCR, AUX_CR | AUX_CS);   /* enable 9914 chip reset state */

	GPIBout(IMR0, 0);                              /* disable all interrupts */
	GPIBout(IMR1, 0);
	s = GPIBin(ISR0);
	s = GPIBin(ISR1);  /* clear status registers by reading */

	GPIBout(ADR,(PAD & LOMASK));                   /* set GPIB address; 
                                                          MTA=PAD|100, MLA=PAD|040*/
	GPIBout(AUXCR, AUX_CR );   /* release 9914 chip reset state */

	GPIBin(DIR);
	GPIBout(AUXCR, AUX_HLDA | AUX_CS); /* Holdoff on all data */

#if defined(HP82335)
#if USEINTS
	ccrbits |= HR_INTEN;
	GPIBout(CCR,ccrbits);
#if 1
	GPIBout(IMR0,0);
	GPIBout(IMR1,0);
	GPIBout(AUXCR,AUX_DAI);
#endif

#endif	
#endif


	DBGout();
	return 1;
}

IBLCL void bdDetach(void)
{
#if defined(HP82335)
	iounmap((void*)ibbase);
#endif
}













