/***************************************************************************
                          ines_write.c  -  description
                             -------------------

    copyright            : (C) 2002 by Frank Mori Hess
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

#include "ines.h"

static const int out_fifo_size = 0xff;

static inline unsigned short num_out_fifo_bytes( ines_private_t *ines_priv )
{
	return inb( iobase( ines_priv ) + OUT_FIFO_COUNT );
}

static int ines_write_wait( gpib_board_t *board, ines_private_t *ines_priv,
	unsigned int fifo_threshold )
{
	nec7210_private_t *nec_priv = &ines_priv->nec7210_priv;

	// wait until byte is ready to be sent
	if( wait_event_interruptible( board->wait,
		num_out_fifo_bytes( ines_priv ) < fifo_threshold ||
		test_bit( BUS_ERROR_BN, &nec_priv->state ) ||
		test_bit( DEV_CLEAR_BN, &nec_priv->state ) ||
		test_bit( TIMO_NUM, &board->status ) ) )
	{
		GPIB_DPRINTK( "gpib write interrupted\n" );
		return -ERESTARTSYS;
	}
	if( test_bit( BUS_ERROR_BN, &nec_priv->state ) )
		return -EIO;
	if( test_bit( DEV_CLEAR_BN, &nec_priv->state ) )
		return -EINTR;
	if( test_bit( TIMO_NUM, &board->status ) )
		return -ETIMEDOUT;

	return 0;
}

ssize_t ines_accel_write( gpib_board_t *board, uint8_t *buffer, size_t length, int send_eoi )
{
	size_t count = 0;
	ssize_t retval = 0;
	ines_private_t *ines_priv = board->private_data;
	nec7210_private_t *nec_priv = &ines_priv->nec7210_priv;
	unsigned int num_bytes, i;

	ines_priv->extend_mode_bits |= XFER_COUNTER_ENABLE_BIT | XFER_COUNTER_OUTPUT_BIT;
	if( send_eoi )
		ines_priv->extend_mode_bits |= LAST_BYTE_HANDLING_BIT;
	else
		ines_priv->extend_mode_bits &= ~LAST_BYTE_HANDLING_BIT;
	outb( ines_priv->extend_mode_bits, iobase( ines_priv ) + EXTEND_MODE );
	ines_set_xfer_counter( ines_priv, length );

	while( count < length )
	{
		retval = ines_write_wait( board, ines_priv, out_fifo_size );
		if( retval < 0 )
			break;

		num_bytes = out_fifo_size - num_out_fifo_bytes( ines_priv );
		if( num_bytes + count > length )
		{
			// should be prevented by transfer counter
			printk( "ines: bug! buffer overflow\n" );
			retval = -EIO;
			break;
		}
		for( i = 0; i < num_bytes; i++ )
		{
			write_byte( nec_priv, buffer[ count++ ], CDOR );
		}
	}
	// wait last byte has been sent
	retval = ines_write_wait( board, ines_priv, 1 );
	if( retval < 0 )
		return retval;

	return length;
}












