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
#include <linux/module.h>
#include <asm/dma.h>

unsigned long ibbase = IBBASE;
unsigned int ibirq = IBIRQ;
unsigned int ibdma = IBDMA;

MODULE_PARM(ibbase, "l");
MODULE_PARM_DESC(ibbase, "base io address");
MODULE_PARM(ibirq, "i");
MODULE_PARM_DESC(ibirq, "interrupt request line");
MODULE_PARM(ibdma, "i");
MODULE_PARM_DESC(ibdma, "dma channel");


// flags to indicate if various resources have been allocated
static unsigned int ioports_allocated = 0, irq_allocated = 0, dma_allocated = 0;

// number of ioports used by NI-AT board
static const unsigned int niat_iosize = 0x20;

// utility function called by board_attach() and board_detach() to put hardware in known, safe state
void board_reset(void)
{
	GPIBout(CMDR, SFTRST);	/* Turbo488 software reset */
	GPIBout(CMDR, CLRSC);		/* by default, disable system controller */
	GPIBout(AUXMR, AUX_CR);	/* 7210 chip reset */
	GPIBout(INTRT, 1);

	GPIBin(CPTR);		/* clear registers by reading */
	GPIBin(ISR1);
	GPIBin(ISR2);

	GPIBout(IMR1, 0);		/* disable all interrupts */
	GPIBout(IMR2, 0);
	GPIBout(SPMR, 0);
	GPIBout(ADR,(PAD & LOMASK));	/* set GPIB address; MTA=PAD|100, MLA=PAD|040 */
#if (SAD)
	GPIBout(ADR, HR_ARS | (SAD & LOMASK)); /* enable secondary addressing */
	GPIBout(ADMR, HR_TRM1 | HR_TRM0 | HR_ADM1);
#else
	GPIBout(ADR, HR_ARS | HR_DT | HR_DL);	/* disable secondary addressing */
	GPIBout(ADMR, HR_TRM1 | HR_TRM0 | HR_ADM0);
#endif
	GPIBout(EOSR, 0);
	GPIBout(AUXMR, ICR | 8);	/* set internal counter register N= 8 */
	GPIBout(AUXMR, PPR | HR_PPU);	/* parallel poll unconfigure */
	GPIBout(AUXMR, auxrabits);
	GPIBout(AUXMR, AUXRB | 0);	/* set INT pin to active high */
	GPIBout(AUXMR, AUXRB | HR_TRI);
	GPIBout(AUXMR, AUXRE | 0);
	GPIBout(TIMER, 0xC4);		/* 0xC4 = 7.5 usec (60 * 0.125) */
}

// initialize hardware
int board_attach(void)
{
	uint8_t		s;
	int		i;

	// nothing is allocated yet
	ioports_allocated = irq_allocated = dma_allocated = 0;

	// allocate ioports
	if(check_region(ibbase, niat_iosize) < 0)
	{
		printk("gpib: ioports are already in use");
		return -1;
	}
	request_region(ibbase, niat_iosize, "gpib");
	ioports_allocated = 1;

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


	s = GPIBpgin(CSR) >> 4;	/* read chip-signature */
	if( s != 0x2 && s!= 0x3)
	{ /* not a NAT488BPL or TNT4882 */
		if (s==0)
		{
			/* Check for old NI card with Turbo 488 & NEC 7210
						from jlavi@cs.joensuu.fi (Jarkko Lavinen) */

			/* Check if FIFO accepts bytes */
			GPIBout(CFG,0);
			GPIBout(CMDR,RSTFIFO);
			GPIBout(CNTL, -16);
			GPIBout(CNTH, -16 >> 8);
			s = GPIBin(ISR3);
			if (s & HR_NEF || !(s & HR_NFF))
			{
				return -1;
			}
			/* FIFO not empty or full or not NI board*/
			for(i=0;i<20;i++)
			{
				GPIBout(FIFOB, (i+42)^(i-488));
				if (! ((s=GPIBin(ISR3)) & HR_NFF)) break;
			}
			/* FIFO should be full exactly after 16 byte writes */
			/* status is 0x15 at this point on my card */
			if (i!=15)
			{
				return -1;
			}
			/* Now read bytes back from FIFO */
			GPIBout(CNTL,-16);
			GPIBout(CNTH,-16 >> 8);
			GPIBout(CFG,C_IN|C_CCEN);
			GPIBout(CMDR,GO);

			for(i = 0; i < 20; i++)
			{
				if ( (uint8_t)((i+42)^(i-488)) != GPIBin(FIFOB))
				{
					return -1;
					/* Fifo not working or not NI board */
				}
				if (!(s=GPIBin(ISR3) & HR_NEF)) break;
			}
			/* FIFO should be empty exactly after 16 byte reads and
				also full due to roundoff */

			if (i != 15 || s!=0)
			{
				return -1;
			}
			/* Ok */
		}else
		{
			return -1;
		}
	}

	board_reset();

	GPIBout(AUXMR, AUX_PON); /* release pon state to bring online */

	return 0;
}

// free resources
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
	if(ioports_allocated)
	{
		board_reset();
		release_region(ibbase, niat_iosize);
		ioports_allocated = 0;
	}
}
