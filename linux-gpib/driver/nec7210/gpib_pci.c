#include "board.h"
#if defined(MODBUS_PCI)

#define INTCSR_DWORD 0x00ff1f00L
#define BMCSR_DWORD  0x08000000L

#define INTCSR_REG    0x38
#define BMCSR_REG     0x3c

#include <linux/pci.h>
#include <asm/io.h>
#include <linux/version.h>
#include <linux/mm.h>

#define MODBUS_VENDOR_ID 0x10b5
#define MODBUS_DEV_ID    0x9050

unsigned int pci_config_reg = 0x0000;
unsigned int pci_status_reg = 0x0000;

struct pci_dev *ib_pci_dev = NULL;

IBLCL void bd_PCIInfo(void)
{
	unsigned long pci_ioaddr[5];

	DBGin("bd_PCIInfo");

	ib_pci_dev = pci_find_device(MODBUS_VENDOR_ID, MODBUS_DEV_ID, NULL);
	if (ib_pci_dev == NULL)
	{
		printk("GPIB: no MODBUS PCI board found\n ");
		return;
	}

	if(pci_enable_device(ib_pci_dev))
	{
		printk("failed to enable pci device\n");
		return;
	}

	pci_ioaddr[0] = ib_pci_dev->resource[0].start;
	pci_ioaddr[1] = ib_pci_dev->resource[1].start;
	pci_ioaddr[2] = ib_pci_dev->resource[2].start;
	pci_ioaddr[3] = ib_pci_dev->resource[3].start;
	pci_ioaddr[4] = ib_pci_dev->resource[4].start;

	pci_ioaddr[0]&= PCI_BASE_ADDRESS_MEM_MASK;
	pci_ioaddr[1]&= PCI_BASE_ADDRESS_IO_MASK;
	pci_ioaddr[2]&= PCI_BASE_ADDRESS_MEM_MASK;
	pci_ioaddr[3]&= PCI_BASE_ADDRESS_MEM_MASK;
	pci_ioaddr[4]&= PCI_BASE_ADDRESS_MEM_MASK;

      printk("GPIB: io0=0x%lx io1=0x%lx io2=0x%lx io3=0x%lx io4=0x%lx \n",
                    pci_ioaddr[0],pci_ioaddr[1],pci_ioaddr[2],pci_ioaddr[3], pci_ioaddr[4] );


//      pci_config_reg = remap_pci_mem( pci_ioaddr[0], 128 ) ;
      ibbase = pci_ioaddr[2];
//     pci_status_reg = remap_pci_mem( pci_ioaddr[4], 0x2000 ) ;

//      printk("GPIB: On Board Reg: 0x%x=0x%x 0x%x=0x%x\n",pci_status_reg+0x1,readb(pci_status_reg+0x1),pci_status_reg+0x3,readb(pci_status_reg+0x3));
//     printk("GPIB: Config Reg: 0x%x=0x%x 0x%x=0x%x\n",pci_config_reg,readb(pci_config_reg),pci_config_reg+1,readb(pci_config_reg+1));

      ibirq  = ib_pci_dev->irq;

      pci_DisableIRQ ();

      printk("GPIB: MODBUS PCI base=0x%lx config=0x%x irq=0x%x \n",ibbase ,pci_config_reg, ibirq );

  DBGout();
}

/* enable or disable PCI interrupt on AMCC PCI controller */

IBLCL void pci_EnableIRQ(void)
{
DBGin("pci_EnableIRQ");
DBGout();
}

IBLCL void pci_ResetIRQ (void)
{
  /*DBGin("pci_ResetIRQ");*/
  /*DBGout();*/
}



IBLCL void pci_DisableIRQ (void)
{
DBGin("pci_DisableIRQ");
DBGout();
}

#endif
