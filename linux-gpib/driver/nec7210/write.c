/***************************************************************************
                              nec7210/write.c
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

ssize_t nec7210_write(uint8_t *buffer, size_t length, int send_eoi)
{
	gpib_char_t data;
	unsigned long flags;
	size_t count = 0;
	int retval = 0;
printk("write %i\n", length);
	if(length == 0) return 0;

	GPIBout(AUXMR, auxa_bits);	/* normal handshaking, XXX necessary?*/

	if(send_eoi)
	{
		length-- ; /* save the last byte for sending EOI */
	}

	if(test_and_set_bit(0, &write_in_progress))
	{
		printk("gpib: bug? write already in progress");
		return -1;
	}

#if DMAOP	// isa dma transfer
	if(length > 0)
	{
		/* program dma controller */
		flags = claim_dma_lock();
		disable_dma(ibdma);
		clear_dma_ff ( ibdma );
		set_dma_count( ibdma, length );
		set_dma_addr ( ibdma, virt_to_bus(buffer));
		set_dma_mode( ibdma, DMA_MODE_WRITE );
		enable_dma(ibdma);
		release_dma_lock(flags);

		// enable board's dma for output
		imr2_bits |= HR_DMAO;
		GPIBout(IMR2, imr2_bits);

		// enable 'data out' interrupts
		imr1_bits |= HR_DOIE;
		GPIBout(IMR1, imr1_bits);

		// suspend until message is sent
		if(wait_event_interruptible(nec7210_wait, test_bit(0, &write_in_progress) == 0))
		{
			printk("gpib write interrupted!\n");
			retval = -1;
		}

		// disable board's dma
		imr2_bits &= ~HR_DMAO;
		GPIBout(IMR2, imr2_bits);

		if(retval == 0) count += length;
	}

#else	// PIO transfer

	data.end = 0;
	if(length > 0)
	{
		// load message into buffer
		while (count < length)
		{
			data.value = buffer[count];
			if(gpib_buffer_put(write_buffer, data))
			{
				printk("gpib: write buffer full!\n");
				return -1;
			}
			count++;
		}

		// enable 'data out' interrupts
		imr1_bits |= HR_DOIE;
		GPIBout(IMR1, imr1_bits);

		// suspend until message is sent
		if(wait_event_interruptible(nec7210_wait, test_bit(0, &write_in_progress) == 0))
		{
			printk("gpib write interrupted!\n");
			retval = -1;
		}
	}

#endif	// DMAOP

	if(send_eoi && retval == 0)
	{
		/*send EOI */
		if((pgmstat & PS_NOEOI) == 0)
			GPIBout(AUXMR, AUX_SEOI);
		set_bit(0, &write_in_progress);
		GPIBout(CDOR, buffer[count]);
		count++;
		wait_event_interruptible(nec7210_wait, test_bit(0, &write_in_progress) == 0);
	}
	// disable 'data out' interrupts
	imr1_bits &= ~HR_DOIE;
	GPIBout(IMR1, imr1_bits);

	if(retval < 0) return retval;
	return count;
}













