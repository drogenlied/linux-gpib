#include <board.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <asm/dma.h>

unsigned long ibbase = IBBASE;	/* base addr of GPIB interface registers  */
unsigned int ibirq  = IBIRQ;	/* interrupt request line for GPIB (1-7)  */
unsigned int ibdma  = IBDMA;     /* DMA channel                            */
unsigned long remapped_ibbase = 0;

MODULE_PARM(ibbase, "l");
MODULE_PARM_DESC(ibbase, "base io address");
MODULE_PARM(ibirq, "i");
MODULE_PARM_DESC(ibirq, "interrupt request line");
MODULE_PARM(ibdma, "i");
MODULE_PARM_DESC(ibdma, "dma channel");

uint8_t       board_type = CBI_ISA_GPIB;
uint8_t       CurHSMode = 0;      /* hs mode register value */
uint8_t       CurIRQreg = 0;      /* hs IRQ register value */

// flags to indicate if various resources have been allocated
static unsigned int ioports_allocated = 0, irq_allocated = 0, dma_allocated = 0, pcmcia_initialized = 0;

// number of ioports cbi boards use (probably underestimate)
static const int cbi_iosize = 0xa;

void board_reset(void)
{
	/* CBI 4882 reset */
	GPIBout(HS_INT_LEVEL, HS_RESET7210 );
	GPIBout(HS_INT_LEVEL, 0 );

	GPIBout(HS_MODE, HS_TX_ENABLE | HS_RX_ENABLE ); /* reset state machine */
	GPIBout(HS_MODE, 0); /* disable system control */
	CurHSMode |= HS_SYS_CONTROL;
	GPIBout(HS_MODE, CurHSMode ); /* enable syscntrl */

        GPIBout(AUXMR, AUX_PON);
	GPIBout(AUXMR, AUX_CR);                     /* 7210 chip reset */
	GPIBout(AUXMR, AUX_CIFC);

	GPIBin(CPTR);                           /* clear registers by reading */

	GPIBin(ISR1);
	GPIBin(ISR2);
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
	GPIBout(AUXMR, ICR | 20);                    /* set internal counter register N= 8 */
	GPIBout(AUXMR, PPR | HR_PPU);               /* parallel poll unconfigure */
	GPIBout(AUXMR, auxrabits);


#if !defined( CBI_PCI )
	GPIBout(AUXMR, AUXRB | 0);                  /* set INT pin to active high */
#else
        GPIBout(AUXMR, AUXRB | HR_INV );           /* On PCI boards set INT pin to active low */
#endif

	GPIBout(AUXMR, AUXRE | 0);
	GPIBout(AUXMR, AUX_LOSPEED );

}
int board_attach(void)
{
	int isr_flags = 0;

	// nothing is allocated yet
	ioports_allocated = irq_allocated = dma_allocated = pcmcia_initialized = 0;

#ifdef CBI_PCMCIA
	pcmcia_init_module();
	pcmcia_initialized = 1;
#endif
#if defined(CBI_PCI)
	bd_PCIInfo();
#endif
	// allocate ioports
	if(check_region(ibbase, cbi_iosize) < 0)
	{
		printk("gpib: ioports are already in use");
		return -1;
	}
	request_region(ibbase, cbi_iosize, "gpib");
	ioports_allocated = 1;

	// install interrupt handler
#if USEINTS
#if defined(CBI_PCI)
	isr_flags |= SA_SHIRQ;
#endif
	if( request_irq(ibirq, ibintr, isr_flags, "gpib", &ibbase))
	{
		printk("gpib: can't request IRQ %d\n", ibirq);
		return -1;
	}
	irq_allocated = 1;
#endif

	// request isa dma channel
#if DMAOP
	if( request_dma( ibdma, "gpib" ) )
	{
		printk("gpib: can't request DMA %d\n",ibdma );
		return -1;
	}
	dma_allocated = 1;
#endif

	/* now its time to check the board type */

	GPIBout( SPMR, 0xaa );
             /* check if serial poll registers are readable & writeable */
        if( GPIBin( SPSR ) != 0xaa ) {
	  printk("GPIB Board is not a CBI488.2! \n");
	  return(0);
	}

        GPIBout( AUXMR, AUX_PAGE + 1 ); /* select paged in register */
        if( GPIBin( SPSR ) != 0xaa ) {  /* does SPMR still return 0xaa ? */
	  DBGprint(DBG_BRANCH, ("    CBI488.2 old type ") );
	}

        if( GPIBin(HS_MODE) == 0xff ) {
	  DBGprint(DBG_BRANCH, (", LC variant\n") );
          board_type = CBI_ISA_GPIB_LC;
	}
	board_reset();
#if defined(CBI_PCI)
	pci_EnableIRQ();
#endif
	GPIBout(AUXMR, AUX_PON);
	return 0;
}

void board_detach(void)
{
	if(dma_allocated)
	{
		free_dma(ibdma);
		dma_allocated = 0;
	}
	if(irq_allocated)
	{
		free_irq(ibirq, 0);
		irq_allocated = 0;
	}
	if(ioports_allocated)
	{
#if defined(CBI_PCI)
		pci_DisableIRQ();
#endif
		board_reset();
		release_region(ibbase, cbi_iosize);
		ioports_allocated = 0;
	}
	if(pcmcia_initialized)
	{
#ifdef CBI_PCMCIA
		pcmcia_cleanup_module();
#endif
		pcmcia_initialized = 0;
	}
}


/*is called by ibsic */

IBLCL void fix4882Bug(void )
{
#if !defined(CBI_PCI)
  int i;
  extern int myPAD;
  char cmdString[8];

   if(board_type == CBI_ISA_GPIB_LC || board_type == CBI_ISA_GPIB ){
   DBGin("fix4882Bug");

        i=0;
#if 0
  	cmdString[i]   = UNL;
        cmdString[i++] = UNT;
#endif
  	cmdString[i] = myPAD | TAD;
        cmdString[i++] = myPAD | LAD;
	if( ibcmd(cmdString, i) & ERR ) {
	  printk("problem fixing bug");
	}
	
	GPIBout(CDOR, 0x20 ); /*send a byte */
	i= GPIBin(DIR);  /*read back*/
	i= GPIBin(ISR1); /*throw away any errors */

        i=0;
  	cmdString[i]   = UNL;
        cmdString[i++] = UNT;
	if( ibcmd(cmdString, i) & ERR ) {
	  printk("problem fixing bug");
	}

	printk("setting IRQ to %d \n",ibirq);
	setup4882int(ibirq);

	DBGout();
   }
#endif
}

IBLCL void setup4882int( int level ) 
{

  DBGin("setup4882Int");

#if !defined(CBI_PCI)
  switch( level ) {
  case 2:
    CurIRQreg |= 1;
    break;
  case 3:
    CurIRQreg |= 2;
    break;
  case 4:
    CurIRQreg |= 3;
    break;
  case 5:
    CurIRQreg |= 4;
    break;
  case 7:
    CurIRQreg |= 5;
    break;
  case 10:
    CurIRQreg |= 6;
    break;
  case 11:
    CurIRQreg |= 7;
    break;
  default:
    printk("IRQ level %d not supported with this board\n",level);
    break;    
  }
  GPIBout(HS_INT_LEVEL, CurIRQreg );
  GPIBout(HS_MODE,      CurHSMode );
#endif
  DBGout();
}

