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

unsigned long ibbase = IBBASE;
unsigned int ibirq = IBIRQ;
unsigned int ibdma = IBDMA;
unsigned long remapped_ibbase = 0;
unsigned long amcc_iobase = 0;

// this is a temporary hack to set the driver pointer
#ifdef NIPCIIa
gpib_driver_t *driver = &pc2a_driver;
#warning using pc2a driver
#endif

#ifdef CBI_PCMCIA
gpib_driver_t *driver = &cb_pcmcia_driver;
#warning using cb_pcmcia driver
#endif

#ifdef CBI_PCI
gpib_driver_t *driver = &cb_pci_driver;
#warning using cb_pci driver
#endif

#if !defined(NIPCIIa) && !defined(CBI_4882)
gpib_driver_t *driver = &pc2_driver;
#warning usin pc2 driver
#endif

MODULE_PARM(ibbase, "l");
MODULE_PARM_DESC(ibbase, "base io address");
MODULE_PARM(ibirq, "i");
MODULE_PARM_DESC(ibirq, "interrupt request line");
MODULE_PARM(ibdma, "i");
MODULE_PARM_DESC(ibdma, "dma channel");

// size of modbus pci memory io region
static const int iomem_size = 0x2000;

void nec7210_board_reset(nec7210_private_t *priv)
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
	return readb(priv->iobase + register_num * priv->offset);
}
// wrapper for writeb
void iomem_write_byte(nec7210_private_t *priv, uint8_t data, unsigned int register_num)
{
	writeb(data, priv->iobase + register_num * priv->offset);
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
}

#endif















