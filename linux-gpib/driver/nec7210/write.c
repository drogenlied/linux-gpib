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
#include <linux/string.h>
#include <asm/dma.h>

static ssize_t pio_write(gpib_board_t *board, nec7210_private_t *priv, uint8_t *buffer, size_t length)
{
	size_t count = 0;
	ssize_t retval = 0;
	unsigned long flags;

	while( count < length )
	{
		spin_lock_irqsave(&board->spinlock, flags);
		clear_bit(WRITE_READY_BN, &priv->state);
		clear_bit(DMA_IN_PROGRESS_BN, &priv->state);
		write_byte(priv, buffer[count++], CDOR);
		spin_unlock_irqrestore(&board->spinlock, flags);

		// wait until byte is ready to be sent
		if( wait_event_interruptible( board->wait,
			test_bit( WRITE_READY_BN, &priv->state) ||
			test_bit( OUTPUT_ERROR_BN, &priv->state ) ||
			test_bit(TIMO_NUM, &board->status ) ) )
		{
			printk("gpib write interrupted\n");
			retval = -EINTR;
			break;
		}
		if( test_and_clear_bit( OUTPUT_ERROR_BN, &priv->state ) )
		{
			count--;
			if( wait_event_interruptible( board->wait,
				gpib_clear_to_write( board ) ||
				test_bit(TIMO_NUM, &board->status ) ) )
			{
				printk("gpib output error wait interrupted\n");
				retval = -EINTR;
				break;
			}
		}
		if( test_bit( TIMO_NUM, &board->status ) )
		{
			retval = -ETIMEDOUT;
			break;
		}
	}

	if(retval)
		return retval;

	return length;
}

static ssize_t __dma_write(gpib_board_t *board, nec7210_private_t *priv, dma_addr_t address, size_t length)
{
	unsigned long flags, dma_irq_flags;
	int residue = 0;
	int retval = 0;

	spin_lock_irqsave(&board->spinlock, flags);

	/* program dma controller */
	dma_irq_flags = claim_dma_lock();
	disable_dma(priv->dma_channel);
	clear_dma_ff(priv->dma_channel);
	set_dma_count(priv->dma_channel, length);
	set_dma_addr(priv->dma_channel, address);
	set_dma_mode(priv->dma_channel, DMA_MODE_WRITE );
	enable_dma(priv->dma_channel);
	release_dma_lock(dma_irq_flags);

	// enable board's dma for output
	priv->imr2_bits |= HR_DMAO;
	write_byte(priv, priv->imr2_bits, IMR2);

	clear_bit(WRITE_READY_BN, &priv->state);
	set_bit(DMA_IN_PROGRESS_BN, &priv->state);

	spin_unlock_irqrestore(&board->spinlock, flags);

	// suspend until message is sent
	if(wait_event_interruptible(board->wait, test_bit(DMA_IN_PROGRESS_BN, &priv->state) == 0 ||
		test_bit(TIMO_NUM, &board->status)))
	{
		printk("gpib write interrupted!\n");
	}
	if(test_bit(TIMO_NUM, &board->status))
		retval = -ETIMEDOUT;

	// disable board's dma
	priv->imr2_bits &= ~HR_DMAO;
	write_byte(priv, priv->imr2_bits, IMR2);

	dma_irq_flags = claim_dma_lock();
	clear_dma_ff(priv->dma_channel);
	disable_dma(priv->dma_channel);
	residue = get_dma_residue(priv->dma_channel);
	release_dma_lock( dma_irq_flags );

	if(residue)
		retval = -EIO;

	return retval ? retval : length;
}

static ssize_t dma_write(gpib_board_t *board, nec7210_private_t *priv, uint8_t *buffer, size_t length)
{
	size_t remain = length;
	size_t transfer_size;
	ssize_t retval = 0;

	while(remain > 0)
	{
		transfer_size = (priv->dma_buffer_length < remain) ? priv->dma_buffer_length : remain;
		memcpy(priv->dma_buffer, buffer, transfer_size);
		retval = __dma_write(board, priv, priv->dma_buffer_addr, transfer_size);
		if(retval < 0) break;
		remain -= retval;
		buffer += retval;
	}

	if(retval < 0) return retval;

	return length - remain;
}

ssize_t nec7210_write(gpib_board_t *board, nec7210_private_t *priv, uint8_t *buffer, size_t length, int send_eoi)
{
	size_t count = 0;
	ssize_t retval = 0;

	if(length == 0) return 0;

	if(send_eoi)
	{
		length-- ; /* save the last byte for sending EOI */
	}

	if(length > 0)
	{
		if(priv->dma_channel)
		{	// isa dma transfer
			retval = dma_write(board, priv, buffer, length);
			if(retval < 0)
				return retval;
			else count += retval;
		}else
		{	// PIO transfer
			retval = pio_write(board, priv, buffer, length);
			if(retval < 0)
				return retval;
			else count += retval;
		}
	}
	if(send_eoi)
	{
		/*send EOI */
		write_byte(priv, AUX_SEOI, AUXMR);

		retval = pio_write(board, priv, &buffer[count], 1);
		if(retval < 0)
			return retval;
		else
			count++;
	}

	return count ? count : -1;
}

EXPORT_SYMBOL(nec7210_write);











