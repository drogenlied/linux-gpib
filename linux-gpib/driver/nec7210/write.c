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

ssize_t nec7210_write(gpib_driver_t *driver, uint8_t *buffer, size_t length, int send_eoi)
{
	gpib_char_t data;
	unsigned long flags;
	size_t count = 0;
	int err = 0;
	nec7210_private_t *priv = driver->private_data;
	
	if(length == 0) return 0;

	/* normal handshaking, XXX necessary?*/
	priv->write_byte(priv, priv->auxa_bits, AUXMR);

	if(send_eoi)
	{
		length-- ; /* save the last byte for sending EOI */
	}

	if(test_and_set_bit(WRITING_BN, &priv->state))
	{
		printk("gpib: bug? write already in progress");
		return -1;
	}

#if DMAOP	// isa dma transfer
	if(length > 0)
	{
		/* program dma controller */
		flags = claim_dma_lock();
		disable_dma(priv->dma_channel);
		clear_dma_ff(priv->dma_channel);
		set_dma_count(priv->dma_channel, length );
		set_dma_addr(priv->dma_channel, virt_to_bus(buffer));
		set_dma_mode(priv->dma_channel, DMA_MODE_WRITE );
		enable_dma(priv->dma_channel);
		release_dma_lock(flags);

		// enable board's dma for output
		priv->imr2_bits |= HR_DMAO;
		priv->write_byte(priv, priv->imr2_bits, IMR2);

		// enable 'data out' interrupts
		priv->imr1_bits |= HR_DOIE;
		priv->write_byte(priv, priv->imr1_bits, IMR1);

		// suspend until message is sent
		if(wait_event_interruptible(driver->wait, test_bit(WRITING_BN, &priv->state) == 0 ||
			test_bit(TIMO_NUM, &driver->status)))
		{
			printk("gpib write interrupted!\n");
		}

		// disable board's dma
		priv->imr2_bits &= ~HR_DMAO;
		priv->write_byte(priv, priv->imr2_bits, IMR2);

		flags = claim_dma_lock();
		clear_dma_ff(priv->dma_channel);
		disable_dma(priv->dma_channel);
		count += length - get_dma_residue(priv->dma_channel);
		release_dma_lock(flags);
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
		}

		// enable 'data out' interrupts
		priv->imr1_bits |= HR_DOIE;
		priv->write_byte(priv, priv->imr1_bits, IMR1);

		// suspend until message is sent
		if(wait_event_interruptible(driver->wait, test_bit(WRITING_BN, &priv->state) == 0 ||
			test_bit(TIMO_NUM, &driver->status)))
		{
			printk("gpib write interrupted!\n");
		}

		// XXX bug if write was interrupted, how is buffer cleaned up?
		count += length - atomic_read(&write_buffer->size);
	}

#endif	// DMAOP

	if(send_eoi && err == 0)
	{
		/*send EOI */
		priv->write_byte(priv, AUX_SEOI, AUXMR);
		set_bit(WRITING_BN, &priv->state);
		priv->write_byte(priv, buffer[count], CDOR);
		wait_event_interruptible(driver->wait, test_bit(WRITING_BN, &priv->state) == 0 ||
			test_bit(TIMO_NUM, &driver->status));
		if(test_and_clear_bit(WRITING_BN, &priv->state) == 0 &&
			test_bit(TIMO_NUM, &driver->status) == 0)
			count++;
	}
	// disable 'data out' interrupts
	priv->imr1_bits &= ~HR_DOIE;
	priv->write_byte(priv, priv->imr1_bits, IMR1);

	return count ? count : -1;
}













