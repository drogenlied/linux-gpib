/*================================================================

  PCI related stuff for ines GPIB-PCI board  
  adapted from ../cbi4882/gpib_pci.c
  13.1.99 Axel Dziemba (axel.dziemba@ines.de)

==================================================================*/

#include "board.h"
#if defined(INES_PCI)

#define INTCSR_DWORD_ENABLE  0x00000063L
#define INTCSR_DWORD_DISABLE 0x00000062L

#define INTCSR_REG    0x4c

#include <linux/pci.h>
#include <linux/bios32.h>
#include <asm/io.h>

#define INES_VENDOR_ID 0x10b5
#define INES_DEV_ID    0x9050
#define INES_SUBID 0x107210b5L


unsigned int pci_config_reg = 0x0000;




void bd_PCIInfo() {
  extern uint16      ibbase;	/* base addr of GPIB interface registers  */
  extern uint8       ibirq;	/* interrupt request line for GPIB (1-7)  */

  DBGin("bd_PCIInfo");
  
  if( pcibios_present() ) {
    int pci_index;
    for(pci_index = 0;pci_index < 8; pci_index++ ) {
      unsigned char pci_bus, pci_device_fn;
      unsigned int pci_ioaddr;
      unsigned int pci_IRQ_line;
      unsigned int pci_subid;

      if ( pcibios_find_device ( INES_VENDOR_ID,
				 INES_DEV_ID, pci_index,
				 &pci_bus, &pci_device_fn ) != 0 )
	{
	  printk("GPIB: no PCI board found\n ");
	  break;
	}
      // check ines subID too!
      pcibios_read_config_dword(pci_bus,pci_device_fn,PCI_SUBSYSTEM_ID,&pci_subid);
      if(pci_subid!=INES_SUBID)
         continue; // board with PLX PCI controller but not ines GPIB PCI board
      pcibios_read_config_byte (pci_bus, pci_device_fn,
			  PCI_INTERRUPT_LINE, &pci_IRQ_line );
      pcibios_read_config_dword ( pci_bus, pci_device_fn,
			  PCI_BASE_ADDRESS_2, &pci_ioaddr );
      pcibios_read_config_dword ( pci_bus, pci_device_fn,
			  PCI_BASE_ADDRESS_1, &pci_config_reg );
      pci_ioaddr &= PCI_BASE_ADDRESS_IO_MASK;
      pci_config_reg &= PCI_BASE_ADDRESS_IO_MASK;
      ibbase = bus_to_virt(pci_ioaddr);
      ibirq = pci_IRQ_line;
      pci_DisableIRQ ();
      printk("GPIB: PCI base=0x%x config=0x%x irq=0x%lx \n",ibbase,pci_config_reg, ibirq );
     break;      
    }
  } 
  DBGout();
}

/* enable or disable PCI interrupt on PLX PCI controller */

void pci_EnableIRQ () {

DBGin("pci_EnableIRQ");
      outl( INTCSR_DWORD_ENABLE, pci_config_reg+INTCSR_REG );
DBGout();

}

void pci_ResetIRQ () {

  DBGin("pci_ResetIRQ");
      outl( INTCSR_DWORD_ENABLE, pci_config_reg+INTCSR_REG );
  DBGout();

}



void pci_DisableIRQ () {

DBGin("pci_DisableIRQ");
     outl( INTCSR_DWORD_DISABLE , pci_config_reg+INTCSR_REG );
DBGout();

}

#endif
