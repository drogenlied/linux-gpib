#include "board.h"
#if defined(MODBUS_PCI)


#define INTCSR_DWORD 0x00ff1f00L
#define BMCSR_DWORD  0x08000000L

#define INTCSR_REG    0x38
#define BMCSR_REG     0x3c




#include <linux/pci.h>
#include <linux/bios32.h>
#include <asm/io.h>
#include <linux/version.h>
#include <linux/mm.h>


typedef     u_long          vm_offset_t; 

#define LinuxVersionCode(v, p, s) (((v)<<16)+((p)<<8)+(s))       

#define MODBUS_VENDOR_ID 0x10b5
#define MODBUS_DEV_ID    0x9050


unsigned int pci_base_reg = 0x0000;
unsigned int pci_config_reg = 0x0000;
unsigned int pci_status_reg = 0x0000;

static vm_offset_t remap_pci_mem(u_long base, u_long size)
{
        u_long page_base        = ((u_long) base) & PAGE_MASK;
        u_long page_offs        = ((u_long) base) - page_base;
#if LINUX_VERSION_CODE >= LinuxVersionCode(2,1,0)
        u_long page_remapped    = (u_long) ioremap(page_base, page_offs+size);
#else
        u_long page_remapped    = (u_long) vremap(page_base, page_offs+size);
#endif

        return (vm_offset_t) (page_remapped ? (page_remapped + page_offs) : 0UL);
}

static void unmap_pci_mem(vm_offset_t vaddr, u_long size)
{
        if (vaddr)
#if LINUX_VERSION_CODE >= LinuxVersionCode(2,1,0)
                iounmap((void *) (vaddr & PAGE_MASK));
#else
                vfree((void *) (vaddr & PAGE_MASK));
#endif
}
 


void bd_PCIInfo() {
  extern uint16      ibbase;	/* base addr of GPIB interface registers  */
  extern uint8       ibirq;	/* interrupt request line for GPIB (1-7)  */


  DBGin("bd_PCIInfo");
  

  if( pcibios_present() ) {
    int pci_index,i;
    for(pci_index = 0;pci_index < 8; pci_index++ ) {
      unsigned char pci_bus, pci_device_fn;
      unsigned int pci_ioaddr0;
      unsigned int pci_ioaddr1;
      unsigned int pci_ioaddr2;
      unsigned int pci_ioaddr3;
      unsigned int pci_ioaddr4;
      unsigned short command;
      unsigned int pci_IRQ_line;

      if ( pcibios_find_device ( MODBUS_VENDOR_ID,
				 MODBUS_DEV_ID, pci_index,
				 &pci_bus, &pci_device_fn ) != 0 )
	{
	  printk("GPIB: no MODBUS PCI board found\n ");
	  break;
	}
		
      pcibios_read_config_byte (pci_bus, pci_device_fn,
			  PCI_INTERRUPT_LINE, &pci_IRQ_line );

      pcibios_read_config_dword ( pci_bus, pci_device_fn,
			  PCI_BASE_ADDRESS_0, &pci_ioaddr0 );
      pcibios_read_config_dword ( pci_bus, pci_device_fn,
			  PCI_BASE_ADDRESS_1, &pci_ioaddr1 );
      pcibios_read_config_dword ( pci_bus, pci_device_fn,
			  PCI_BASE_ADDRESS_2, &pci_ioaddr2 );
      pcibios_read_config_dword ( pci_bus, pci_device_fn,
			  PCI_BASE_ADDRESS_3, &pci_ioaddr3 );
      pcibios_read_config_dword ( pci_bus, pci_device_fn,
			  PCI_BASE_ADDRESS_4, &pci_ioaddr4 );
      pcibios_read_config_word( pci_bus, pci_device_fn,
			PCI_COMMAND, &command	);

      pci_ioaddr0     &= PCI_BASE_ADDRESS_MEM_MASK;
      pci_ioaddr1     &= PCI_BASE_ADDRESS_IO_MASK;
      pci_ioaddr2     &= PCI_BASE_ADDRESS_MEM_MASK;
      pci_ioaddr3     &= PCI_BASE_ADDRESS_MEM_MASK;
      pci_ioaddr4     &= PCI_BASE_ADDRESS_MEM_MASK;

      printk("GPIB: io0=0x%lx io1=0x%lx io2=0x%lx io3=0x%lx io4=0x%lx \n",
                    pci_ioaddr0,pci_ioaddr1,pci_ioaddr2,pci_ioaddr3, pci_ioaddr4 );


      pci_config_reg = remap_pci_mem( pci_ioaddr0, 128 ) ;
      pci_base_reg   = remap_pci_mem( pci_ioaddr2, 0x2000 ) ;
      pci_status_reg = remap_pci_mem( pci_ioaddr4, 0x2000 ) ;
      
      printk("GPIB: On Board Reg: 0x%x=0x%x 0x%x=0x%x\n",pci_status_reg+0x1,readb(pci_status_reg+0x1),pci_status_reg+0x3,readb(pci_status_reg+0x3));
      printk("GPIB: Config Reg: 0x%x=0x%x 0x%x=0x%x\n",pci_config_reg,readb(pci_config_reg),pci_config_reg+1,readb(pci_config_reg+1));
      
      ibbase = 0x000;
      ibirq  = pci_IRQ_line;

      writeb( 0xff, (pci_base_reg+ibbase+0x20)); /* enable controller mode */

      pci_DisableIRQ ();

      printk("GPIB: MODBUS PCI base=0x%x config=0x%x irq=0x%lx \n",pci_base_reg ,pci_config_reg, ibirq );
      break;
      
  
    }

  } 

  DBGout();
}


bdPCIDetach() {
DBGin("bdPCIDetach");
      unmap_pci_mem( pci_config_reg, 128 ) ;
      unmap_pci_mem( pci_base_reg,   0x2000 ) ;
      unmap_pci_mem( pci_status_reg, 0x2000 ) ;
DBGout();
}







/* enable or disable PCI interrupt on AMCC PCI controller */

void pci_EnableIRQ () {
DBGin("pci_EnableIRQ");
/*
      outl( BMCSR_DWORD,  pci_config_reg+BMCSR_REG );
      SLOW_DOWN_IO;
      outl( INTCSR_DWORD, pci_config_reg+INTCSR_REG );
      */
DBGout();
}

void pci_ResetIRQ () {
  /*DBGin("pci_ResetIRQ");*/
  /*
      outl( INTCSR_DWORD, pci_config_reg+INTCSR_REG );
      SLOW_DOWN_IO;
      outl( BMCSR_DWORD,  pci_config_reg+BMCSR_REG );
      */
  /*DBGout();*/
}



void pci_DisableIRQ () {
DBGin("pci_DisableIRQ");
/*
     outl( 0x00ff0000 , pci_config_reg+INTCSR_REG );
     SLOW_DOWN_IO;
     outl( BMCSR_DWORD,  pci_config_reg+BMCSR_REG );
     */
DBGout();
}

#endif
