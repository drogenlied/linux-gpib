#include "board.h"
#if defined(CBI_PCI)

#include <linux/pci.h>
#include <asm/io.h>

#define INTCSR_DWORD 0x00ff1f00L
#define BMCSR_DWORD  0x08000000L

#define INTCSR_REG    0x38
#define BMCSR_REG     0x3c

#define CBI_VENDOR_ID 0x1307
#define CBI_DEV_ID 0x6

unsigned int pci_config_reg = 0x0000;

struct pci_dev *ib_pci_dev = NULL;

IBLCL void bd_PCIInfo(void)
{
	DBGin("bd_PCIInfo");

	ib_pci_dev = pci_find_device(CBI_VENDOR_ID, CBI_DEV_ID, NULL);
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

	pci_config_reg = ib_pci_dev->resource[0].start & PCI_BASE_ADDRESS_IO_MASK;
	ibbase = ib_pci_dev->resource[1].start & PCI_BASE_ADDRESS_IO_MASK;
	ibirq = ib_pci_dev->irq;

	pci_DisableIRQ ();

	printk("GPIB: PCI base=0x%lx config=0x%x irq=0x%x \n",ibbase,pci_config_reg, ibirq );

	DBGout();
}

/* enable or disable PCI interrupt on AMCC PCI controller */

IBLCL void pci_EnableIRQ(void)
{
	DBGin("pci_EnableIRQ");
	outl_p( BMCSR_DWORD,  pci_config_reg+BMCSR_REG );
	outl( INTCSR_DWORD, pci_config_reg+INTCSR_REG );
	DBGout();
}

IBLCL void pci_ResetIRQ(void)
{
  /*DBGin("pci_ResetIRQ");*/
      outl_p( INTCSR_DWORD, pci_config_reg+INTCSR_REG );
      outl( BMCSR_DWORD,  pci_config_reg+BMCSR_REG );
  /*DBGout();*/
}



IBLCL void pci_DisableIRQ(void)
{
DBGin("pci_DisableIRQ");
     outl_p( 0x00ff0000 , pci_config_reg+INTCSR_REG );
     outl( BMCSR_DWORD,  pci_config_reg+BMCSR_REG );
DBGout();
}

#endif




