

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
	int		i;           
	extern uint32	osRegAddr();

	DBGin("bdonl");

        /*printk("base=0x%lx\n",ibbase);*/

#if defined(ZIATECH)
	ib = (struct ibregs *) ( ibbase );          /* setting base address */
	printk("Ziatech: set base to 0x%p \n ",ib);
	printk("Ziatech: isr1 = 0x%p \n ",&IB->isr1);
	printk("Ziatech: adswr= 0x%p \n ",&IB->adswr);
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
	     printk("hp82335 base range 0x%x invalid, see Hardware Manual\n",ibbase);
             DBGout(); return(0);
           break;
	}


        if( ibirq < 3 || ibirq > 7 ){
	  printk("Illegal Interrupt Level must be within 3..7\n");
          DBGout(); return(0);
	}


	ib = (struct ibregs *) ( ibbase <<4 );          /* setting base address */
        /*printk("ib=0x%x\n",ib);*/

#if 0
        s = GPIBin( csr );
        printk("csr(0x%lx)=0x%x\n",&IB->csr,s);

        s = GPIBin( hpibsr );
        printk("hpibsr(0x%lx)=0x%x\n",&IB->hpibsr,s);
                /* clear registers by reading */

        printk("testing for hp27209 \n");
        GPIBout( ccr, 0 );                 
        s = GPIBin( csr );
        printk("csr(0x%lx)=0x%x\n",&IB->csr,s);

	s = GPIBin(isr0);
        printk("isr0(0x%lx)=0x%x\n",&IB->isr0,s);
	s = GPIBin(isr1);
        printk("isr1(0x%lx)=0x%x\n",&IB->isr1,s);
	s = GPIBin(adsr);
        printk("adsr(0x%lx)=0x%x\n",&IB->adsr,s);
#endif
#endif

	GPIBout(auxcr, AUX_CR | AUX_CS);   /* enable 9914 chip reset state */

	GPIBout(imr0, 0);                              /* disable all interrupts */
	GPIBout(imr1, 0);
	s = GPIBin(isr0);
	s = GPIBin(isr1);  /* clear status registers by reading */

	GPIBout(adr,(PAD & LOMASK));                   /* set GPIB address; 
                                                          MTA=PAD|100, MLA=PAD|040*/
#if 0
#if (SAD)
	GPIBout(adr, HR_EDPA | (SAD & LOMASK));        /* enable secondary addressing */
#else
	GPIBout(adr, HR_EDPA | HR_DAT | HR_DAL);       /* disable secondary addressing */
#endif
#endif

	GPIBout(auxcr, AUX_CR );   /* release 9914 chip reset state */

#if 1
	GPIBin(dir);
	GPIBout(auxcr, AUX_HLDA | AUX_CS); /* Holdoff on all data */
#endif

#if defined(HP82335)
#if USEINTS
	ccrbits |= HR_INTEN;
	GPIBout(ccr,ccrbits);
#if 1
	GPIBout(imr0,0);
	GPIBout(imr1,0);
	GPIBout(auxcr,AUX_DAI);
#endif

#endif	
#endif


	DBGout();
	return(1);
}














