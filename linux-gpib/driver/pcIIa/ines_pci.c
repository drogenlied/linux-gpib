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
#include <asm/io.h>

#define INES_VENDOR_ID 0x10b5
#define INES_DEV_ID    0x9050
#define INES_SUBID 0x107210b5L


unsigned int pci_config_reg = 0x0000;




void bd_PCIInfo() {

	DBGin("bd_PCIInfo");

	ib_pci_dev = NULL;
	while((ib_pci_dev = pci_find_device(INES_VENDOR_ID, INES_DEV_ID, ib_pci_dev)))
	{
		// check for board with PLX PCI controller but not ines GPIB PCI board
		if(ib_pci_dev->subsystem_device == INES_SUBID)
		{
			break;
		}
	}
	if(ib_pci_dev == NULL)
	{
		printk("GPIB: no PCI board found\n ");
		return;
	}
	if(pci_enable_device(ib_pci_dev))
	{
		printk("error enabling pci device\n");
		return;
	}

	ibbase = ib_pci_dev->resource[2].start & PCI_BASE_ADDRESS_IO_MASK;

	ibirq = ib_pci_dev->irq;
	pci_DisableIRQ ();
	printk("GPIB: PCI base=0x%lx config=0x%x irq=0x%x \n",ibbase,pci_config_reg, ibirq );

	DBGout();
}

/* enable or disable PCI interrupt on PLX PCI controller */

IBLCL void pci_EnableIRQ (void)
{

DBGin("pci_EnableIRQ");
      outl( INTCSR_DWORD_ENABLE, pci_config_reg+INTCSR_REG );
DBGout();

}

IBLCL void pci_ResetIRQ (void)
{

  DBGin("pci_ResetIRQ");
      outl( INTCSR_DWORD_ENABLE, pci_config_reg+INTCSR_REG );
  DBGout();

}



IBLCL void pci_DisableIRQ (void)
{

DBGin("pci_DisableIRQ");
     outl( INTCSR_DWORD_DISABLE , pci_config_reg+INTCSR_REG );
DBGout();

}

#endif
