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

int output_fifo_empty( const cb7210_private_t *cb_priv )
{
	const nec7210_private_t *nec_priv = &cb_priv->nec7210_priv;

	if( ( inb( nec_priv->iobase + HS_STATUS ) & ( HS_TX_MSB_EMPTY | HS_TX_LSB_EMPTY ) ) ==
		( HS_TX_MSB_EMPTY | HS_TX_LSB_EMPTY ) )
		return 1;
	else
		return 0;
}

static inline void output_fifo_enable( cb7210_private_t *cb_priv, int enable )
{
	nec7210_private_t *nec_priv = &cb_priv->nec7210_priv;

	if( enable )
	{
		nec7210_set_reg_bits( nec_priv, IMR2, HR_DMAO, 1 );

		cb_priv->hs_mode_bits &= ~HS_ENABLE_MASK;
		cb_priv->hs_mode_bits |= HS_TX_ENABLE;
		outb( cb_priv->hs_mode_bits, nec_priv->iobase + HS_MODE );
	}else
	{
		cb_priv->hs_mode_bits &= ~HS_ENABLE_MASK;
		outb( cb_priv->hs_mode_bits, nec_priv->iobase + HS_MODE );

		nec7210_set_reg_bits( nec_priv, IMR2, HR_DMAO, 0 );
	}
}

ssize_t cb7210_accel_write( gpib_board_t *board, uint8_t *buffer, size_t length, int send_eoi )
{
	size_t count = 0;
	ssize_t retval = 0;
	cb7210_private_t *cb_priv = board->private_data;
	nec7210_private_t *nec_priv = &cb_priv->nec7210_priv;
	unsigned long iobase = nec_priv->iobase;
	unsigned int num_bytes, i;

	output_fifo_enable( cb_priv, 1 );

	while( count < length )
	{

		num_bytes = cb7210_fifo_size / 2;
		if( num_bytes + count > length )
			num_bytes = length - count;
		for( i = 0; i < num_bytes / 2; i++ )
		{
			uint16_t word;

			word = buffer[ count++ ] & 0xff;
			if( count < length )
				word |= ( buffer[ count++ ] << 8 ) & 0xff00;
			outw( word, iobase + CDOR );
		}
		cb_priv->out_fifo_half_empty = 0;
		// wait until byte is ready to be sent
		if( wait_event_interruptible( board->wait,
			cb_priv->out_fifo_half_empty ||
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

	output_fifo_enable( cb_priv, 0 );

	if( retval < 0 )
		return retval;

	return length;
}












