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

#define PCI_DEVICE_ID_CBOARDS_PCI_GPIB 0x6

int pc2_attach(void);
int pc2a_attach(void);
int pnp_attach(void);
int cb_pci_attach(void);

void pc2_detach(void);
void cb_pci_detach(void);

unsigned long ibbase = IBBASE;
unsigned int ibirq = IBIRQ;
unsigned int ibdma = IBDMA;
unsigned long remapped_ibbase = 0;

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
	status:	0,
	private_data:	NULL,
};

gpib_driver_t pc2a_driver =
{
	name:	"nec7210",
	attach:	pc2a_attach,
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
	status:	0,
	private_data:	NULL,
};

gpib_driver_t pnp_driver =
{
	attach:	pnp_attach,
};

// this is a hack to set the driver pointer
#ifdef NIPCIIa
gpib_driver_t *driver = &pc2a_driver;
#warning using pc2a driver
#endif

#ifdef CBI_PCI
gpib_driver_t *driver = &pnp_driver;
#warning using pnp driver
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

// flags to indicate if various resources have been allocated
static unsigned int ioports_allocated = 0, iomem_allocated = 0,
	irq_allocated = 0, dma_allocated = 0, pcmcia_initialized = 0;

// bits written to interrupt mask registers
volatile int imr1_bits, imr2_bits;
/* bits written to auxillary register A, excluding handshaking bits.  Used to
 * hold EOS information */
int auxa_bits = AUXRA;

// nec7210 has 8 registers
static const int nec7210_num_registers = 8;
// size of modbus pci memory io region
static const int iomem_size = 0x2000;

void board_reset(void)
{
#ifdef MODBUS_PCI
	GPIBout(0x20, 0xff); /* enable controller mode */
#endif

	GPIBout(AUXMR, AUX_CR);                     /* 7210 chip reset */

	GPIBin(CPTR);                           /* clear registers by reading */
	GPIBin(ISR1);
	GPIBin(ISR2);

	GPIBout(IMR1, 0);                           /* disable all interrupts */
	GPIBout(IMR2, 0);
	GPIBout(SPMR, 0);

	GPIBout(EOSR, 0);
	GPIBout(AUXMR, ICR | 8);                    /* set internal counter register N= 8 */
	GPIBout(AUXMR, PPR | HR_PPU);               /* parallel poll unconfigure */

	GPIBout(ADR, (PAD & ADDRESS_MASK));                /* set GPIB address; MTA=PAD|100, MLA=PAD|040 */
	admr_bits = HR_TRM0 | HR_TRM1;
#if (SAD)
	GPIBout(ADR, HR_ARS | (SAD & ADDRESS_MASK));      /* enable secondary addressing */
	admr_bits |= HR_ADM1;
	GPIBout(ADMR, admr_bits);
#else
	GPIBout(ADR, HR_ARS | HR_DT | HR_DL);       /* disable secondary addressing */
	admr_bits |= HR_ADM0;
	GPIBout(ADMR, admr_bits);
#endif

	// holdoff on all data	XXX record current handshake state somewhere
	auxa_bits = AUXRA;
	GPIBout(AUXMR, auxa_bits | HR_HLDA);

	GPIBout(AUXMR, AUXRB);                  /* set INT pin to active high */
	GPIBout(AUXMR, AUXRE);
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

int pc2_attach(void)
{
	unsigned int i, err;
	int isr_flags = 0;

	// nothing is allocated yet
	ioports_allocated = iomem_allocated = irq_allocated =
		dma_allocated = pcmcia_initialized = 0;

	if(allocate_buffers())
		return -1;

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

	// install interrupt handler
	if( request_irq(ibirq, nec7210_interrupt, isr_flags, "gpib", &ibbase))
	{
		printk("gpib: can't request IRQ %d\n", ibirq);
		return -1;
	}
	irq_allocated = 1;

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

void pc2_detach(void)
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
	if(ioports_allocated)
	{
		board_reset();
		for(i = 0; i < nec7210_num_registers; i++)
			release_region(ibbase + i * NEC7210_REG_OFFSET, 1);
		ioports_allocated = 0;
	}
	free_buffers();
}

int pc2a_attach(void)
{
	unsigned int i, err;
	int isr_flags = 0;

	// nothing is allocated yet
	ioports_allocated = iomem_allocated = irq_allocated =
		dma_allocated = pcmcia_initialized = 0;

	if(allocate_buffers())
		return -1;

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

	if( request_irq(ibirq, pc2a_interrupt, isr_flags, "gpib", &ibbase))
	{
		printk("gpib: can't request IRQ %d\n", ibirq);
		return -1;
	}
	irq_allocated = 1;
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

	// make sure interrupt is clear
        outb(0xff , CLEAR_INTR_REG(ibirq));

	// enable interrupts
	imr1_bits = HR_ERRIE | HR_DECIE | HR_ENDIE |
		HR_DETIE | HR_APTIE | HR_CPTIE;
	imr2_bits = IMR2_ENABLE_INTR_MASK;
	GPIBout(IMR1, imr1_bits);
	GPIBout(IMR2, imr2_bits);

	GPIBout(AUXMR, AUX_PON);

	return 0;
}

// this function will do more once I support more than one pnp board
int pnp_attach(void)
{

	driver->name = "nec7210";
	driver->detach = cb_pci_detach;
	driver->read = nec7210_read;
	driver->write = nec7210_write;
	driver->command = nec7210_command;
	driver->take_control = nec7210_take_control;
	driver->go_to_standby = nec7210_go_to_standby;
	driver->interface_clear = nec7210_interface_clear;
	driver->remote_enable = nec7210_remote_enable;
	driver->enable_eos = nec7210_enable_eos;
	driver->disable_eos = nec7210_disable_eos;
	driver->parallel_poll = nec7210_parallel_poll;
	driver->line_status = NULL;	//XXX
	driver->update_status = nec7210_update_status;
	driver->primary_address = nec7210_primary_address;
	driver->secondary_address = nec7210_secondary_address;
	driver->serial_poll_response = nec7210_serial_poll_response;
	driver->status = 0;
	driver->private_data = NULL;

	return cb_pci_attach();
}

int cb_pci_attach(void)
{
	unsigned int i, err;
	int isr_flags = 0;
	int bits;

	// nothing is allocated yet
	ioports_allocated = iomem_allocated = irq_allocated =
		dma_allocated = pcmcia_initialized = 0;

	// find board
	pci_dev_ptr = pci_find_device(PCI_VENDOR_ID_CBOARDS, PCI_DEVICE_ID_CBOARDS_PCI_GPIB, NULL);
	if(pci_dev_ptr == NULL)
	{
		printk("GPIB: no PCI-GPIB board found\n");
		return -1;
	}

	if(pci_enable_device(pci_dev_ptr))
	{
		printk("error enabling pci device\n");
		return -1;
	}

	amcc_iobase = pci_resource_start(pci_dev_ptr, 0) & PCI_BASE_ADDRESS_IO_MASK;
	ibbase = pci_resource_start(pci_dev_ptr, 1) & PCI_BASE_ADDRESS_IO_MASK;
	ibirq = pci_dev_ptr->irq;

	if(allocate_buffers())
		return -1;

	if(pci_request_regions(pci_dev_ptr, "pci-gpib"))
		return -1;
	ioports_allocated = 1;

	/* CBI 4882 reset */
	GPIBout(HS_INT_LEVEL, HS_RESET7210 );
	GPIBout(HS_MODE, 0); /* disable system control */

	// use register page 0 for nec7210 compatibility
	GPIBout(AUXMR, AUX_PAGE);

	// set clock register for 20MHz driving frequency
	GPIBout(AUXMR, ICR | 8);

	isr_flags |= SA_SHIRQ;
	if(request_irq(ibirq, cb_pci_interrupt, isr_flags, "gpib", &ibbase))
	{
		printk("gpib: can't request IRQ %d\n", ibirq);
		return -1;
	}
	irq_allocated = 1;

	board_reset();

	// enable interrupts on amccs5933 chip
	bits = INBOX_FULL_INTR_BIT | INBOX_BYTE_BITS(3) | INBOX_SELECT_BITS(3) |
		INBOX_INTR_CS_BIT;
	outl(bits, amcc_iobase + INTCSR_REG );

	// enable interrupts for cb7210
	imr1_bits = HR_ERRIE | HR_DECIE | HR_ENDIE |
		HR_DETIE | HR_APTIE | HR_CPTIE;
	imr2_bits = IMR2_ENABLE_INTR_MASK;
	GPIBout(IMR1, imr1_bits);
	GPIBout(IMR2, imr2_bits);

	GPIBout(AUXMR, AUX_PON);

	return 0;
}

void cb7210_detach(void)
{
	int i;
	if(ioports_allocated)
	{
		// disable amcc interrupts
		outl(0, amcc_iobase + INTCSR_REG );
	}
	if(irq_allocated)
	{
		free_irq(ibirq, &ibbase);
		irq_allocated = 0;
	}
	if(ioports_allocated)
	{
		board_reset();
		pci_release_regions(pci_dev_ptr);
		ioports_allocated = 0;
	}
	free_buffers();
}

// old functions
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
	if( request_irq(ibirq, nec7210_interrupt, isr_flags, "gpib", &ibbase))
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

















