/***************************************************************************
                              nec7210/read.c
                             -------------------

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
#include <asm/dma.h>

ssize_t nec7210_read(uint8_t *buffer, size_t length, int *end)
{
	size_t	count = 0;
	gpib_char_t data;
	int ret;
	unsigned long flags;

	*end = 0;

	if(length == 0) return 0;

	if (pgmstat & PS_HELD) {
		GPIBout(AUXMR, auxa_bits | HR_HLDA);
		GPIBout(AUXMR, AUX_FH);	/* set HLDA in AUXRA to ensure FH works */
		pgmstat &= ~PS_HELD;
	}
/*
 *	holdoff on END
 */
	GPIBout(AUXMR, auxa_bits | HR_HLDE);

#if DMAOP		// ISA DMA transfer
	flags = claim_dma_lock();
	disable_dma(ibdma);

	/* program dma controller */
	clear_dma_ff ( ibdma );
	set_dma_count( ibdma, length );
	set_dma_addr ( ibdma, virt_to_bus(buffer));
	set_dma_mode( ibdma, DMA_MODE_READ );
	release_dma_lock(flags);

	enable_dma(ibdma);

	// enable nec7210 dma
	imr2_bits |= HR_DMAI;
	GPIBout(IMR2, imr2_bits);

	// wait for data to transfer
	if(wait_event_interruptible(nec7210_read_wait, test_and_clear_bit(0, &dma_transfer_complete)))
	{
		printk("read wait interrupted\n");
		return -1;
	}

	// disable nec7210 dma
	imr2_bits &= ~HR_DMAI;
	GPIBout(IMR2, imr2_bits);

	// record how many bytes we transferred
	flags = claim_dma_lock();
	clear_dma_ff ( ibdma );
	disable_dma(ibdma);
	count += length - get_dma_residue(ibdma);
	release_dma_lock(flags);

#else	// PIO transfer

	// enable 'data in' interrupt
	imr1_bits |= HR_DIIE;
	GPIBout(IMR1, imr1_bits);

	while (count < length )
	{
		ret = gpib_buffer_get(read_buffer, &data);
		if(ret < 0)
		{
			if(wait_event_interruptible(nec7210_read_wait, atomic_read(&read_buffer->size) > 0))
			{
				printk("wait failed\n");
				// XXX
				break;
			};
			continue;
		}
		buffer[count++] = data.value;
		if(data.end)
		{
			set_bit(END_NUM, &board.status);
			break;
		}
	}

	// disable 'data in' interrupt
	imr1_bits &= ~HR_DIIE;
	GPIBout(IMR1, imr1_bits);

#endif

	pgmstat |= PS_HELD;
	GPIBout(AUXMR, auxa_bits | HR_HLDA);

	if (!noTimo)
	{
		set_bit(ERR_NUM, &board.status);
		set_bit(TIMO_NUM, &board.status);
		iberr = EABO;
	}

	if(test_bit(END_NUM, &board.status))
		*end = 1;

	return count;
}








