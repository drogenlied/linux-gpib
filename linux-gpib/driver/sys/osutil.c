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

IBLCL void osChngBase(unsigned long new_base)
{
  DBGin("osChngBase");

#if !defined(CBI_PCMCIA) && !defined(CBI_PCI) && !defined(MODBUS_PCI) && !defined(INES_PCMCIA) && !defined(INES_PCI)
  if( !(pgmstat & PS_SYSRDY ) && ibbase != new_base ){
      printk("GPIB: Change Base from 0x%lx to 0x%lx\n",ibbase,new_base);
      ibbase = new_base;
  }
#else
      printk("GPIB: PCMCIA/PCI base address: 0x%lx\n",ibbase);
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


