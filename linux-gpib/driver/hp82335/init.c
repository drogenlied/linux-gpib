/***************************************************************************
                          init.c  -  description
                             -------------------
 board specific initialization stuff

    begin                : Dec 2001
    copyright            : (C) 2001 by Frank Mori Hess, and unknown author(s)
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
#include <asm/dma.h>

unsigned long ibbase = IBBASE;
uint8 ibirq = IBIRQ;
uint8 ibdma = IBDMA;

unsigned long remapped_ibbase = 0;

// flags to indicate if various resources have been allocated
static unsigned int ioports_allocated = 0, iomem_allocated = 0, irq_allocated = 0, dma_allocated = 0;

static const int iomem_size = 0x4000;
static const int io_size = 0x8;	// ziatech uses at least 8 ports

void board_reset(void)
{
	GPIBout(AUXCR, AUX_CR | AUX_CS);   /* enable 9914 chip reset state */

	GPIBout(IMR0, 0);                              /* disable all interrupts */
	GPIBout(IMR1, 0);
	GPIBin(ISR0);
	GPIBin(ISR1);  /* clear status registers by reading */

	GPIBout(ADR,(PAD & LOMASK));                   /* set GPIB address;
                                                          MTA=PAD|100, MLA=PAD|040*/
	GPIBout(AUXCR, AUX_CR );   /* release 9914 chip reset state */

	GPIBin(DIR);
	GPIBout(AUXCR, AUX_HLDA | AUX_CS); /* Holdoff on all data */

#if defined(HP82335) && USEINTS
	ccrbits |= HR_INTEN;
	GPIBout(CCR,ccrbits);
	GPIBout(IMR0,0);
	GPIBout(IMR1,0);
	GPIBout(AUXCR,AUX_DAI);
#endif
}

int board_attach(void)
{
	// nothing is allocated yet
	ioports_allocated = iomem_allocated = irq_allocated = dma_allocated = 0;

#if defined(ZIATECH)
	printk("Ziatech: set base to 0x%lx \n ", ibbase);
	printk("Ziatech: ISR1 = 0x%lx \n ", ibbase + ISR1);
	printk("Ziatech: adswr= 0x%lx \n ", ibbase + ADSWR);
#endif
#if defined(HP82335)
	switch( ibbase )
	{
		case 0xC000:
		case 0xC400:
		case 0xC800:
		case 0xCC00:
		case 0xD000:
		case 0xD400:
		case 0xD800:
		case 0xDC00:
		case 0xE000:
		case 0xE400:
		case 0xE800:
		case 0xEC00:
		case 0xF000:
		case 0xF400:
		case 0xF800:
		case 0xFC00:
		break;
	default:
		printk("hp82335 base range 0x%lx invalid, see Hardware Manual\n",ibbase);
		return -1;
		break;
	}
        if( ibirq < 3 || ibirq > 7 )
	{
		printk("Illegal Interrupt Level must be within 3..7\n");
		return -1;
	}
	if(check_mem_region(ibbase, iomem_size))
	{
		printk("gpib: memory io region already in use");
		return -1;
	}
	request_mem_region(ibbase, iomem_size, "gpib");
	remapped_ibbase = (unsigned long) ioremap(ibbase, iomem_size);          /* setting base address */
	iomem_allocated = 1;
#else
	if(check_region(ibbase, iosize))
	{
		printk("gpib: ioports are already in use");
		return -1;
	}
	request_region(ibbase, io_size, "gpib");
	ioports_allocated = 1;
#endif
	// install interrupt handler
#if USEINTS
	if( request_irq(ibirq, ibintr, 0, "gpib", NULL))
	{
		printk("gpib: can't request IRQ %d\n", ibirq);
		return -1;
	}
	irq_allocated = 1;
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
	return 0;
}

void board_detach(void)
{
	if(dma_allocated)
	{
		free_dma(ibdma);
		dma_allocated = 0;
	}
	if(irq_allocated)
	{
		free_irq(ibirq, 0);
		irq_allocated = 0;
	}
	if(ioports_allocated || iomem_allocated)
	{
		board_reset();
	}
	if(ioports_allocated)
	{
		release_region(ibbase, io_size);
		ioports_allocated = 0;
	}
	if(iomem_allocated)
	{
		iounmap((void*) remapped_ibbase);
		release_mem_region(ibbase, iomem_size);
		iomem_allocated = 0;
	}
}














