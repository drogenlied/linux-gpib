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

static int pio_write_wait( gpib_board_t *board, nec7210_private_t *priv )
{
	// wait until byte is ready to be sent
	if( wait_event_interruptible( board->wait,
		test_bit( WRITE_READY_BN, &priv->state ) ||
		test_bit( BUS_ERROR_BN, &priv->state ) ||
		test_bit( DEV_CLEAR_BN, &priv->state ) ||
		test_bit( TIMO_NUM, &board->status ) ) )
	{
		GPIB_DPRINTK( "gpib write interrupted\n" );
		return -ERESTARTSYS;
	}
	if( test_bit( TIMO_NUM, &board->status ) )
		return -ETIMEDOUT;
	if( test_bit( DEV_CLEAR_BN, &priv->state ) )
		return -EINTR;
	if( test_bit( BUS_ERROR_BN, &priv->state ) )
		return -EIO;

	return 0;
}


static ssize_t pio_write(gpib_board_t *board, nec7210_private_t *priv, uint8_t *buffer, size_t length)
{
	size_t count = 0;
	ssize_t retval = 0;
	unsigned long flags;

	while( count < length )
	{
		retval = pio_write_wait( board, priv );
		if( retval < 0 ) return retval;

		spin_lock_irqsave(&board->spinlock, flags);
		clear_bit(WRITE_READY_BN, &priv->state);
		write_byte(priv, buffer[ count++ ], CDOR);
		spin_unlock_irqrestore(&board->spinlock, flags);

		if( current->need_resched )
			schedule();
	}
	retval = pio_write_wait( board, priv );
	if( retval < 0 ) return retval;

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
	nec7210_set_reg_bits( priv, IMR2, HR_DMAO, HR_DMAO );

	clear_bit(WRITE_READY_BN, &priv->state);
	set_bit(DMA_WRITE_IN_PROGRESS_BN, &priv->state);

	spin_unlock_irqrestore(&board->spinlock, flags);

	// suspend until message is sent
	if(wait_event_interruptible(board->wait, test_bit(DMA_WRITE_IN_PROGRESS_BN, &priv->state) == 0 ||
		test_bit( BUS_ERROR_BN, &priv->state ) || test_bit( DEV_CLEAR_BN, &priv->state ) ||
		test_bit(TIMO_NUM, &board->status)))
	{
		GPIB_DPRINTK( "gpib write interrupted!\n" );
		retval = -ERESTARTSYS;
	}
	if(test_bit(TIMO_NUM, &board->status))
		retval = -ETIMEDOUT;
	if( test_and_clear_bit( DEV_CLEAR_BN, &priv->state ) )
		retval = -EINTR;
	if( test_and_clear_bit( BUS_ERROR_BN, &priv->state ) )
		retval = -EIO;

	// disable board's dma
	nec7210_set_reg_bits( priv, IMR2, HR_DMAO, 0 );

	dma_irq_flags = claim_dma_lock();
	clear_dma_ff(priv->dma_channel);
	disable_dma(priv->dma_channel);
	residue = get_dma_residue(priv->dma_channel);
	release_dma_lock( dma_irq_flags );

	if(residue)
		retval = -EPIPE;

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

	clear_bit( DEV_CLEAR_BN, &priv->state );
	clear_bit( BUS_ERROR_BN, &priv->state );

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











