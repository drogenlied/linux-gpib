/***************************************************************************
                          nec7210/init.c  -  description
                             -------------------
 board specific initialization stuff

    begin                : Dec 2001
    copyright            : (C) 2001, 2002 by Frank Mori Hess
    email                : fmhess@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "board.h"
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <asm/dma.h>
#include <gpib_buffer.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/string.h>

#define PCI_DEVICE_ID_CBOARDS_PCI_GPIB 0x6

int pc2_attach(gpib_driver_t *driver);
int pc2a_attach(gpib_driver_t *driver);
int pnp_attach(gpib_driver_t *driver);
int cb_pci_attach(gpib_driver_t *driver);

void pc2_detach(gpib_driver_t *driver);
void pc2a_detach(gpib_driver_t *driver);
void cb_pci_detach(gpib_driver_t *driver);

unsigned long ibbase = IBBASE;
unsigned int ibirq = IBIRQ;
unsigned int ibdma = IBDMA;
unsigned long remapped_ibbase = 0;
unsigned long amcc_iobase = 0;

struct pci_dev *pci_dev_ptr = NULL;

gpib_driver_t pc2_driver =
{
	name:	"nec7210",
	attach:	pc2_attach,
	detach:	pc2_detach,
	read:	nec7210_read,
	write:	nec7210_write,
	command:	nec7210_command,
	take_control:	nec7210_take_control,
	go_to_standby:	nec7210_go_to_standby,
	interface_clear:	nec7210_interface_clear,
	remote_enable:	nec7210_remote_enable,
	enable_eos:	nec7210_enable_eos,
	disable_eos:	nec7210_disable_eos,
	parallel_poll:	nec7210_parallel_poll,
	line_status:	NULL,
	update_status:	nec7210_update_status,
	primary_address:	nec7210_primary_address,
	secondary_address:	nec7210_secondary_address,
	serial_poll_response:	nec7210_serial_poll_response,
};

gpib_driver_t pc2a_driver =
{
	name:	"nec7210",
	attach:	pc2a_attach,
	detach:	pc2a_detach,
	read:	nec7210_read,
	write:	nec7210_write,
	command:	nec7210_command,
	take_control:	nec7210_take_control,
	go_to_standby:	nec7210_go_to_standby,
	interface_clear:	nec7210_interface_clear,
	remote_enable:	nec7210_remote_enable,
	enable_eos:	nec7210_enable_eos,
	disable_eos:	nec7210_disable_eos,
	parallel_poll:	nec7210_parallel_poll,
	line_status:	NULL,
	update_status:	nec7210_update_status,
	primary_address:	nec7210_primary_address,
	secondary_address:	nec7210_secondary_address,
	serial_poll_response:	nec7210_serial_poll_response,
};

gpib_driver_t cb_pci_driver =
{
	name: "nec7210",
	attach: cb_pci_attach,
	detach: cb_pci_detach,
	read: nec7210_read,
	write: nec7210_write,
	command: nec7210_command,
	take_control: nec7210_take_control,
	go_to_standby: nec7210_go_to_standby,
	interface_clear: nec7210_interface_clear,
	remote_enable: nec7210_remote_enable,
	enable_eos: nec7210_enable_eos,
	disable_eos: nec7210_disable_eos,
	parallel_poll: nec7210_parallel_poll,
	line_status: NULL,	//XXX
	update_status: nec7210_update_status,
	primary_address: nec7210_primary_address,
	secondary_address: nec7210_secondary_address,
	serial_poll_response: nec7210_serial_poll_response,
};

// this is a hack to set the driver pointer
#ifdef NIPCIIa
gpib_driver_t *driver = &pc2a_driver;
#warning using pc2a driver
#endif

#ifdef CBI_PCI
gpib_driver_t *driver = &cb_pci_driver;
#warning using cb_pci driver
#endif

#if !defined(NIPCIIa) && !defined(CBI_4882)
gpib_driver_t *driver = &pc2_driver;
#warning usin pc2 driver
#endif

gpib_buffer_t *read_buffer = NULL, *write_buffer = NULL;

MODULE_PARM(ibbase, "l");
MODULE_PARM_DESC(ibbase, "base io address");
MODULE_PARM(ibirq, "i");
MODULE_PARM_DESC(ibirq, "interrupt request line");
MODULE_PARM(ibdma, "i");
MODULE_PARM_DESC(ibdma, "dma channel");

// size of modbus pci memory io region
static const int iomem_size = 0x2000;

void board_reset(nec7210_private_t *priv)
{
#ifdef MODBUS_PCI
	GPIBout(0x20, 0xff); /* enable controller mode */
#endif

	priv->write_byte(priv, AUX_CR, AUXMR);                     /* 7210 chip reset */

	priv->read_byte(priv, CPTR);                           /* clear registers by reading */
	priv->read_byte(priv, ISR1);
	priv->read_byte(priv, ISR2);

	/* disable all interrupts */
	priv->imr1_bits = 0;
	priv->write_byte(priv, priv->imr1_bits, IMR1);
	priv->imr2_bits = 0;
	priv->write_byte(priv, priv->imr2_bits, IMR2);
	priv->write_byte(priv, 0, SPMR);

	priv->write_byte(priv, 0, EOSR);
	/* set internal counter register 8 for 8 MHz input clock */
	priv->write_byte(priv, ICR + 8, AUXMR);                    /* set internal counter register N= 8 */
	priv->write_byte(priv, PPR | HR_PPU, AUXMR);               /* parallel poll unconfigure */

	priv->write_byte(priv, PAD & ADDRESS_MASK, ADR);                /* set GPIB address; MTA=PAD|100, MLA=PAD|040 */
	priv->admr_bits = HR_TRM0 | HR_TRM1;
#if (SAD)
	priv->write_byte(priv, HR_ARS | (SAD & ADDRESS_MASK), ADR);      /* enable secondary addressing */
	priv->admr_bits |= HR_ADM1;
	priv->write_byte(priv, priv->admr_bits, ADMR);
#else
	priv->write_byte(priv, HR_ARS | HR_DT | HR_DL, ADR);       /* disable secondary addressing */
	priv->admr_bits |= HR_ADM0;
	priv->write_byte(priv, priv->admr_bits, ADMR);
#endif

	// holdoff on all data	XXX record current handshake state somewhere
	priv->auxa_bits = AUXRA;
	priv->write_byte(priv, priv->auxa_bits | HR_HLDA, AUXMR);

	priv->write_byte(priv, AUXRB, AUXMR);                  /* set INT pin to active high */
	priv->write_byte(priv, AUXRE, AUXMR);
}

int allocate_buffers(void)
{
	read_buffer = kmalloc(sizeof(gpib_buffer_t), GFP_KERNEL);
	gpib_buffer_init(read_buffer);
	write_buffer = kmalloc(sizeof(gpib_buffer_t), GFP_KERNEL);
	gpib_buffer_init(write_buffer);
	if(read_buffer == NULL || write_buffer == NULL)
	{
		printk("gpib: failed to allocate buffers\n");
		return -1;
	}
	return 0;
}

void free_buffers(void)
{
	if(read_buffer)
	{
		kfree(read_buffer);
		read_buffer = NULL;
	}
	if(write_buffer)
	{
		kfree(write_buffer);
		write_buffer = NULL;
	}
}

int allocate_private(gpib_driver_t *driver)
{
	driver->private_data = kmalloc(sizeof(nec7210_private_t), GFP_KERNEL);
	if(driver->private_data == NULL)
		return -1;
	memset(driver->private_data, 0, sizeof(nec7210_private_t));
	return 0;
}

void free_private(gpib_driver_t *driver)
{
	if(driver->private_data)
	{
		kfree(driver->private_data);
		driver->private_data = NULL;
	}
}

// wrapper for inb
uint8_t ioport_read_byte(nec7210_private_t *priv, unsigned int register_num)
{
	return inb(priv->iobase + register_num * priv->offset);
}
// wrapper for outb
void ioport_write_byte(nec7210_private_t *priv, uint8_t data, unsigned int register_num)
{
	outb(data, priv->iobase + register_num * priv->offset);
}

// wrapper for readb
uint8_t iomem_read_byte(nec7210_private_t *priv, unsigned int register_num)
{
	return readb(priv->remapped_iobase + register_num * priv->offset);
}
// wrapper for writeb
void iomem_write_byte(nec7210_private_t *priv, uint8_t data, unsigned int register_num)
{
	writeb(data, priv->remapped_iobase + register_num * priv->offset);
}

int pc2_attach(gpib_driver_t *driver)
{
	int isr_flags = 0;
	nec7210_private_t *priv;
	driver->status = 0;

	if(allocate_private(driver))
		return -ENOMEM;
	priv = driver->private_data;
	priv->offset = pc2_reg_offset;
	priv->read_byte = ioport_read_byte;
	priv->write_byte = ioport_write_byte;

	if(allocate_buffers())
		return -ENOMEM;

	if(request_region(ibbase, pc2_iosize, "pc2"));
	{
		printk("gpib: ioports are already in use");
		return -1;
	}
	priv->iobase = ibbase;

	// install interrupt handler
	if( request_irq(ibirq, nec7210_interrupt, isr_flags, "pc2", driver))
	{
		printk("gpib: can't request IRQ %d\n", ibirq);
		return -1;
	}
	priv->irq = ibirq;

	// request isa dma channel
#if DMAOP
	if( request_dma( ibdma, "pc2" ) )
	{
		printk("gpib: can't request DMA %d\n",ibdma );
		return -1;
	}
	priv->dma_channel = ibdma;
#endif
	board_reset(priv);

	// enable interrupts
	priv->imr1_bits = HR_ERRIE | HR_DECIE | HR_ENDIE |
		HR_DETIE | HR_APTIE | HR_CPTIE;
	priv->imr2_bits = IMR2_ENABLE_INTR_MASK;
	priv->write_byte(priv, priv->imr1_bits, IMR1);
	priv->write_byte(priv, priv->imr2_bits, IMR2);

	priv->write_byte(priv, AUX_PON, AUXMR);

	return 0;
}

void pc2_detach(gpib_driver_t *driver)
{
	nec7210_private_t *priv = driver->private_data;

	if(priv)
	{
		if(priv->dma_channel)
		{
			free_dma(priv->dma_channel);
		}
		if(priv->irq)
		{
			free_irq(priv->irq, driver);
		}
		if(priv->iobase)
		{
			board_reset(priv);
			release_region(priv->iobase, pc2_iosize);
		}
	}
	free_buffers();
	free_private(driver);
}

int pc2a_attach(gpib_driver_t *driver)
{
	unsigned int i, err;
	int isr_flags = 0;
	nec7210_private_t *priv;

	driver->status = 0;

	if(allocate_private(driver))
		return -ENOMEM;
	priv = driver->private_data;
	priv->offset = pc2a_reg_offset;
	priv->read_byte = ioport_read_byte;
	priv->write_byte = ioport_write_byte;

	if(allocate_buffers())
		return -ENOMEM;

	switch( ibbase ){

		case 0x02e1:
		case 0x22e1:
		case 0x42e1:
		case 0x62e1:
			break;
		default:
			printk("PCIIa base range invalid, must be one of [0246]2e1 is %lx \n", ibbase);
			return -1;
			break;
	}

	if( ibirq < 2 || ibirq > 7 )
	{
		printk("Illegal Interrupt Level \n");
		return -1;
	}

	err = 0;
	for(i = 0; i < nec7210_num_registers; i++)
	{
		if(check_region(ibbase + i * pc2a_reg_offset, 1))
			err++;
	}
	if(check_region(pc2a_clear_intr_iobase, pc2a_clear_intr_iosize))
	{
		err++;
	}
	if(err)
	{
		printk("gpib: ioports are already in use");
		return -1;
	}
	for(i = 0; i < nec7210_num_registers; i++)
	{
		request_region(ibbase + i * pc2a_reg_offset, 1, "pc2a");
	}
	request_region(pc2a_clear_intr_iobase, pc2a_clear_intr_iosize, "pc2a");
	priv->iobase = ibbase;

	if(request_irq(ibirq, pc2a_interrupt, isr_flags, "pc2a", driver))
	{
		printk("gpib: can't request IRQ %d\n", ibirq);
		return -1;
	}
	priv->irq = ibirq;
	// request isa dma channel
#if DMAOP
	if(request_dma(ibdma, "pc2a"))
	{
		printk("gpib: can't request DMA %d\n",ibdma );
		return -1;
	}
	priv->dma_channel = ibdma;
#endif
	board_reset(priv);

	// make sure interrupt is clear
	outb(0xff , CLEAR_INTR_REG(ibirq));

	// enable interrupts
	priv->imr1_bits = HR_ERRIE | HR_DECIE | HR_ENDIE |
		HR_DETIE | HR_APTIE | HR_CPTIE;
	priv->imr2_bits = IMR2_ENABLE_INTR_MASK;
	priv->write_byte(priv, priv->imr1_bits, IMR1);
	priv->write_byte(priv, priv->imr2_bits, IMR2);

	priv->write_byte(priv, AUX_PON, AUXMR);

	return 0;
}

void pc2a_detach(gpib_driver_t *driver)
{
	int i;
	nec7210_private_t *priv = driver->private_data;

	if(priv->dma_channel)
	{
		free_dma(priv->dma_channel);
	}
	if(priv->irq)
	{
		free_irq(priv->irq, driver);
	}
	if(priv->iobase)
	{
		board_reset(priv);
		for(i = 0; i < nec7210_num_registers; i++)
			release_region(priv->iobase + i * pc2a_reg_offset, 1);
		release_region(pc2a_clear_intr_iobase, pc2a_clear_intr_iosize);
	}
	free_buffers();
	free_private(driver);
}

int cb_pci_attach(gpib_driver_t *driver)
{
	nec7210_private_t *priv;
	int isr_flags = 0;
	int bits;

	driver->status = 0;

	if(allocate_private(driver))
		return -ENOMEM;
	priv = driver->private_data;
	priv->read_byte = ioport_read_byte;
	priv->write_byte = ioport_write_byte;
	priv->offset = cb_pci_reg_offset;

	if(allocate_buffers())
		return -ENOMEM;

	// find board
	priv->pci_device = pci_find_device(PCI_VENDOR_ID_CBOARDS, PCI_DEVICE_ID_CBOARDS_PCI_GPIB, NULL);
	if(priv->pci_device == NULL)
	{
		printk("GPIB: no PCI-GPIB board found\n");
		return -1;
	}

	if(pci_enable_device(priv->pci_device))
	{
		printk("error enabling pci device\n");
		return -1;
	}

	if(pci_request_regions(priv->pci_device, "pci-gpib"))
		return -1;

	//XXX global
	amcc_iobase = pci_resource_start(pci_dev_ptr, 0) & PCI_BASE_ADDRESS_IO_MASK;
	priv->iobase = pci_resource_start(pci_dev_ptr, 1) & PCI_BASE_ADDRESS_IO_MASK;

	/* CBI 4882 reset */
	priv->write_byte(priv, HS_RESET7210, HS_INT_LEVEL);
	priv->write_byte(priv, 0, HS_INT_LEVEL);
	priv->write_byte(priv, 0, HS_MODE); /* disable system control */

	isr_flags |= SA_SHIRQ;
	if(request_irq(priv->pci_device->irq, cb_pci_interrupt, isr_flags, "pci-gpib", driver))
	{
		printk("gpib: can't request IRQ %d\n", ibirq);
		return -1;
	}
	priv->irq = priv->pci_device->irq;

	board_reset(priv);

	// XXX set clock register for 20MHz? driving frequency
	priv->write_byte(priv, ICR | 8, AUXMR);

	// enable interrupts on amccs5933 chip
	bits = INBOX_FULL_INTR_BIT | INBOX_BYTE_BITS(3) | INBOX_SELECT_BITS(3) |
		INBOX_INTR_CS_BIT;
	outl(bits, amcc_iobase + INTCSR_REG );

	// enable interrupts for cb7210
	priv->imr1_bits = HR_ERRIE | HR_DECIE | HR_ENDIE |
		HR_DETIE | HR_APTIE | HR_CPTIE;
	priv->imr2_bits = IMR2_ENABLE_INTR_MASK;
	priv->write_byte(priv, priv->imr1_bits, IMR1);
	priv->write_byte(priv, priv->imr2_bits, IMR2);

	priv->write_byte(priv, AUX_PON, AUXMR);

	return 0;
}

void cb_pci_detach(gpib_driver_t *driver)
{
	nec7210_private_t *priv = driver->private_data;
	if(priv)
	{
		if(priv->irq)
		{
			// disable amcc interrupts
			outl(0, amcc_iobase + INTCSR_REG );
			free_irq(priv->irq, driver);
		}
		if(priv->iobase)
		{
			board_reset(priv);
			pci_release_regions(priv->pci_device);
		}
	}
	free_buffers();
	free_private(driver);
}

// old functions
#if 0
int board_attach(void)
{
	unsigned int i, err;
	int isr_flags = 0;

	// nothing is allocated yet
	ioports_allocated = iomem_allocated = irq_allocated =
		dma_allocated = pcmcia_initialized = 0;

	if(allocate_buffers())
		return -1;
#ifdef INES_PCMCIA
	pcmcia_init_module();
	pcmcia_initialized = 1;
#endif
#if defined(MODBUS_PCI) || defined(INES_PCI)
   bd_PCIInfo();
#endif

#ifdef NIPCIIa
	switch( ibbase ){

		case 0x02e1:
		case 0x22e1:
		case 0x42e1:
		case 0x62e1:
			break;
	   default:
	     printk("PCIIa base range invalid, must be one of [0246]2e1 is %lx \n", ibbase);
             return(0);
           break;
	}

        if( ibirq < 2 || ibirq > 7 ){
	  printk("Illegal Interrupt Level \n");
          return(0);
	}
#endif
#ifdef MODBUS_PCI
	// modbus uses io memory instead of ioports
	if(check_mem_region(ibbase, iomem_size))
	{
		printk("gpib: memory io region already in use");
		return -1;
	}
	request_mem_region(ibbase, iomem_size, "gpib");
	remapped_ibbase = (unsigned long) ioremap(ibbase, iomem_size);
	iomem_allocated = 1;
#else
	/* nec7210 registers can be spread out to varying degrees, so allocate
	 * each one seperately.  Some boards have extra registers that I haven't
	 * bothered to reserve.  fmhess */
	err = 0;
	for(i = 0; i < nec7210_num_registers; i++)
	{
		if(check_region(ibbase + i * NEC7210_REG_OFFSET, 1))
			err++;
	}
	if(err)
	{
		printk("gpib: ioports are already in use");
		return -1;
	}
	for(i = 0; i < nec7210_num_registers; i++)
	{
		request_region(ibbase + i * NEC7210_REG_OFFSET, 1, "gpib");
	}
	ioports_allocated = 1;
#endif
	// install interrupt handler
#if USEINTS
#if defined(MODBUS_PCI) || defined(INES_PCI)
	isr_flags |= SA_SHIRQ;
#endif
	if( request_irq(ibirq, nec7210_interrupt, isr_flags, "gpib", driver))
	{
		printk("gpib: can't request IRQ %d\n", ibirq);
		return -1;
	}
	irq_allocated = 1;
#if defined(MODBUS_PCI) || defined(INES_PCI)
	pci_EnableIRQ();
#endif
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
	board_reset();

	// enable interrupts
	imr1_bits = HR_ERRIE | HR_DECIE | HR_ENDIE |
		HR_DETIE | HR_APTIE | HR_CPTIE;
	imr2_bits = IMR2_ENABLE_INTR_MASK;
	GPIBout(IMR1, imr1_bits);
	GPIBout(IMR2, imr2_bits);

	GPIBout(AUXMR, AUX_PON);

	return 0;
}

void board_detach(void)
{
	int i;
	if(dma_allocated)
	{
		free_dma(ibdma);
		dma_allocated = 0;
	}
	if(irq_allocated)
	{
		free_irq(ibirq, &ibbase);
		irq_allocated = 0;
	}
	if(ioports_allocated || iomem_allocated)
	{
		board_reset();
#if defined(MODBUS_PCI) || defined(INES_PCI)
		pci_DisableIRQ();
#endif
	}
	if(ioports_allocated)
	{
		for(i = 0; i < nec7210_num_registers; i++)
			release_region(ibbase + i * NEC7210_REG_OFFSET, 1);
		ioports_allocated = 0;
	}
	if(iomem_allocated)
	{
		iounmap((void*) remapped_ibbase);
		release_mem_region(ibbase, iomem_size);
		iomem_allocated = 0;
	}
	if(pcmcia_initialized)
	{
#ifdef INES_PCMCIA
		pcmcia_cleanup_module();
#endif
		pcmcia_initialized = 0;
	}
	free_buffers();
}

#endif















