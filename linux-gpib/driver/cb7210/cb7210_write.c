/***************************************************************************
                             cb7210_write.c
                             -------------------

    copyright            : (C) 2003 by Frank Mori Hess
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

#include "cb7210.h"
#include <linux/delay.h>

int output_fifo_empty( const cb7210_private_t *cb_priv )
{
	const nec7210_private_t *nec_priv = &cb_priv->nec7210_priv;

	if( ( inb( nec_priv->iobase + HS_STATUS ) & ( HS_TX_MSB_NOT_EMPTY | HS_TX_LSB_NOT_EMPTY ) ) == 0 )
		return 1;
	else
		return 0;
}

static inline void output_fifo_enable( gpib_board_t *board, int enable )
{
	cb7210_private_t *cb_priv = board->private_data;
	nec7210_private_t *nec_priv = &cb_priv->nec7210_priv;
	unsigned long flags;

	spin_lock_irqsave( &board->spinlock, flags );

	if( enable )
	{
		nec7210_set_reg_bits( nec_priv, IMR1, HR_DOIE, 0 );
		nec7210_set_reg_bits( nec_priv, IMR2, HR_DMAO, HR_DMAO );

		outb( HS_RX_ENABLE | HS_TX_ENABLE | HS_CLR_SRQ_INT |
			HS_CLR_EOI_EMPTY_INT | HS_CLR_HF_INT, nec_priv->iobase + HS_MODE );

		cb_priv->hs_mode_bits &= ~HS_ENABLE_MASK;
		cb_priv->hs_mode_bits |= HS_TX_ENABLE;
		outb( cb_priv->hs_mode_bits, nec_priv->iobase + HS_MODE );

		outb( irq_bits( cb_priv->irq ), nec_priv->iobase + HS_INT_LEVEL );

		clear_bit( WRITE_READY_BN, &nec_priv->state );

	}else
	{
		cb_priv->hs_mode_bits &= ~HS_ENABLE_MASK;
		outb( cb_priv->hs_mode_bits, nec_priv->iobase + HS_MODE );

		nec7210_set_reg_bits( nec_priv, IMR2, HR_DMAO, 0 );
		nec7210_set_reg_bits( nec_priv, IMR1, HR_DOIE, HR_DOIE );
	}

	spin_unlock_irqrestore( &board->spinlock, flags );
}

ssize_t fifo_write( gpib_board_t *board, uint8_t *buffer, size_t length )
{
	size_t count = 0;
	ssize_t retval = 0;
	cb7210_private_t *cb_priv = board->private_data;
	nec7210_private_t *nec_priv = &cb_priv->nec7210_priv;
	unsigned long iobase = nec_priv->iobase;
	unsigned int num_bytes, i;
	unsigned long flags;

	if( length == 0 ) return 0;

	output_fifo_enable( board, 1 );

	while( count < length )
	{
		// wait until byte is ready to be sent
		if( wait_event_interruptible( board->wait,
			cb_priv->out_fifo_half_empty ||
			output_fifo_empty( cb_priv ) ||
			test_bit( TIMO_NUM, &board->status ) ) )
		{
			printk("gpib write interrupted\n");
			retval = -ERESTARTSYS;
			break;
		}
		if( test_bit( TIMO_NUM, &board->status ) )
		{
			retval = -ETIMEDOUT;
			break;
		}

		if( output_fifo_empty( cb_priv ) )
			num_bytes = cb7210_fifo_size - cb7210_fifo_width;
		else num_bytes = cb7210_fifo_size / 2;
		if( num_bytes + count > length )
			num_bytes = length - count;
		if( num_bytes % cb7210_fifo_width )
		{
			printk( "cb7210: bug! fifo_write() with odd number of bytes\n");
			retval = -EIO;
			break;
		}

		spin_lock_irqsave( &board->spinlock, flags );
		for( i = 0; i < num_bytes / cb7210_fifo_width; i++ )
		{
			uint16_t word;

			word = buffer[ count++ ] & 0xff;
			word |= ( buffer[ count++ ] << 8 ) & 0xff00;
			outw( word, iobase + CDOR );
		}
		cb_priv->out_fifo_half_empty = 0;
		outb( cb_priv->hs_mode_bits | HS_CLR_EOI_EMPTY_INT | HS_CLR_HF_INT, nec_priv->iobase + HS_MODE );
		outb( cb_priv->hs_mode_bits, nec_priv->iobase + HS_MODE );
		spin_unlock_irqrestore( &board->spinlock, flags );
	}
	// wait last byte has been sent
	if( wait_event_interruptible( board->wait,
		output_fifo_empty( cb_priv ) ||
		test_bit( TIMO_NUM, &board->status ) ) )
	{
		printk("gpib write interrupted\n");
		retval = -ERESTARTSYS;
	}
	if( test_bit( TIMO_NUM, &board->status ) )
	{
		retval = -ETIMEDOUT;
	}

	output_fifo_enable( board, 0 );

	if( retval < 0 )
		return retval;

	return count;
}

ssize_t cb7210_accel_write( gpib_board_t *board, uint8_t *buffer, size_t length, int send_eoi )
{
	cb7210_private_t *cb_priv = board->private_data;
	nec7210_private_t *nec_priv = &cb_priv->nec7210_priv;
	unsigned long fast_chunk_size, leftover;
	long retval;

	if( length > cb7210_fifo_width )
		fast_chunk_size = length - 1;
	else
		fast_chunk_size = 0;
	fast_chunk_size -= fast_chunk_size % cb7210_fifo_width;
	leftover = length - fast_chunk_size;

	retval = fifo_write( board, buffer, fast_chunk_size );
	if( retval < 0 ) return retval;
	if( retval < fast_chunk_size ) return -EIO;
	
	retval = nec7210_write( board, nec_priv, buffer + fast_chunk_size, leftover, send_eoi );
	if( retval < 0 ) return retval;

	return length;
}










