#define COMPILING_SYS_OSUTIL_C

#include <ibsys.h>

/*
 * Send EOI to the Interrupt controller
 */ 
#ifdef NIPCIIa
IBLCL void osSendEOI(void)
{
  cli();
  outb(0x20,0xa0);
  outb(0x20,0x20);  /* SEND EOI */
  sti();
}
#else
IBLCL void osSendEOI(void)
{
}
#endif
/*
 * Input a one-byte value from the specified I/O port
 */
IBLCL uint8 osP8in(short in_addr)
{
	uint8		retval;

	retval = inb_p(in_addr);

	DBGprint(DBG_REG, ("0x%x^p8_%x  ", retval, in_addr));

	return retval;
}


/*
 * Output a one-byte value to the specified I/O port
 */
IBLCL void osP8out(short out_addr,uint8 out_value)	
{

	DBGprint(DBG_REG, ("p8_%x^0x%x  ", out_addr, out_value));

	outb_p(out_value,out_addr);

}

/*
 * Input a two-byte value from the specified I/O port
 */
IBLCL uint16 osP16in(short in_addr)
{
	uint16		retval;

	retval = inw_p(in_addr);

	DBGprint(DBG_REG, ("0x%x^p16_%x  ", retval, in_addr));
	return retval;
}


/*
 * Output a two-byte value to the specified I/O port
 */
IBLCL void osP16out(short out_addr, uint16 out_value)	
{
	DBGprint(DBG_REG, ("p16_%x^0x%x  ", out_addr, out_value));

	outw_p(out_value,out_addr);

}


void osPrint(fmt, a, b, c, d, e, f, g)
char *fmt;
{
#if DEBUG
	  if (fmt){
	    printk(fmt, a, b, c, d, e, f, g);
	  }
	  else if ( dbgMask & DBG_1PPL)
	    printk("\n");
#else
	  printk(fmt, a, b, c, d, e, f, g);
#endif
	
}



IBLCL void osChngBase(int new_base)
{
  DBGin("osChngBase");

#if !defined(CBI_PCMCIA) && !defined(CBI_PCI) && !defined(MODBUS_PCI) && !defined(INES_PCMCIA) && !defined(INES_PCI) 
  if( !(pgmstat & PS_SYSRDY ) && ibbase != new_base ){
      printk("GPIB: Change Base from 0x%x to 0x%x\n",ibbase,new_base);
      ibbase = new_base;
  }
#else
      printk("GPIB: PCMCIA/PCI base address: 0x%x\n",ibbase);
#endif
  DBGout();
}


IBLCL void osChngIRQ(int new_irq)
{
  DBGin("osChngIRQ");

#if !defined(CBI_PCMCIA) && !defined(CBI_PCI) && !defined(MODBUS_PCI) && !defined(INES_PCMCIA) && !defined(INES_PCI) 
  if( !(pgmstat & PS_SYSRDY ) && ibirq != new_irq ){
      printk("GPIB: Change IRQ from %d to %d\n",ibirq,new_irq);
      ibirq = new_irq;
  }
#else
  printk("GPIB: PCMCIA/PCI IRQ setting: %d\n",ibirq);

#endif
  DBGout();
}


/*
 *  Change DMA channel
 *
 *
 */


IBLCL void osChngDMA(int new_dma)
{
  DBGin("osChngDMA");


  if( !(pgmstat & PS_SYSRDY ) && ibdma != new_dma ){
      printk("GPIB: Change DMA from %d to %d\n",ibdma,new_dma);
      ibdma = new_dma;
  }
  DBGout();
}


IBLCL uint32 osRegAddr(faddr_t io_addr)
{
	faddr_t		local_addr;	/* local address of GPIB interface */

	DBGin("osRegAddr");
	local_addr = io_addr;
	DBGprint(DBG_DATA, ("localAddr=0x%x  ", local_addr));
	DBGout();
	return ((uint32) local_addr);
}






