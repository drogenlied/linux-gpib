#include "board.h"
#if defined(CBI_PCI)


#define INTCSR_DWORD 0x00ff1f00L
#define BMCSR_DWORD  0x08000000L

#define INTCSR_REG    0x38
#define BMCSR_REG     0x3c




#include <linux/pci.h>
#include <linux/bios32.h>
#include <asm/io.h>

#define CBI_VENDOR_ID 0x1307
#define CBI_DEV_ID 6


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

      if ( pcibios_find_device ( CBI_VENDOR_ID,
				 CBI_DEV_ID, pci_index,
				 &pci_bus, &pci_device_fn ) != 0 )
	{
	  printk("GPIB: no PCI board found\n ");
	  break;
	}
		
      pcibios_read_config_byte (pci_bus, pci_device_fn,
			  PCI_INTERRUPT_LINE, &pci_IRQ_line );
      pcibios_read_config_dword ( pci_bus, pci_device_fn,
			  PCI_BASE_ADDRESS_1, &pci_ioaddr );
      pcibios_read_config_dword ( pci_bus, pci_device_fn,
			  PCI_BASE_ADDRESS_0, &pci_config_reg );

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

/* enable or disable PCI interrupt on AMCC PCI controller */

void pci_EnableIRQ () {
DBGin("pci_EnableIRQ");
      outl( BMCSR_DWORD,  pci_config_reg+BMCSR_REG );
      SLOW_DOWN_IO;
      outl( INTCSR_DWORD, pci_config_reg+INTCSR_REG );
DBGout();
}

void pci_ResetIRQ () {
  /*DBGin("pci_ResetIRQ");*/
      outl( INTCSR_DWORD, pci_config_reg+INTCSR_REG );
      SLOW_DOWN_IO;
      outl( BMCSR_DWORD,  pci_config_reg+BMCSR_REG );
  /*DBGout();*/
}



void pci_DisableIRQ () {
DBGin("pci_DisableIRQ");
     outl( 0x00ff0000 , pci_config_reg+INTCSR_REG );
     SLOW_DOWN_IO;
     outl( BMCSR_DWORD,  pci_config_reg+BMCSR_REG );
DBGout();
}

#endif




