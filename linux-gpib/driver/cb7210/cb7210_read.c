/***************************************************************************
                             cb7210_read.c
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

static inline int have_fifo_word( const cb7210_private_t *cb_priv )
{
	const nec7210_private_t *nec_priv = &cb_priv->nec7210_priv;

	if( ( ( inb( nec_priv->iobase + HS_STATUS ) ) &
			( HS_RX_MSB_NOT_EMPTY | HS_RX_LSB_NOT_EMPTY ) ) ==
			( HS_RX_MSB_NOT_EMPTY | HS_RX_LSB_NOT_EMPTY ) )
		return 1;
	else
		return 0;
}

static inline void input_fifo_enable( gpib_board_t *board, int enable )
{
	cb7210_private_t *cb_priv = board->private_data;
	nec7210_private_t *nec_priv = &cb_priv->nec7210_priv;
	unsigned long flags;

	spin_lock_irqsave( &board->spinlock, flags );

	if( enable )
	{
		outb( HS_RX_ENABLE | HS_TX_ENABLE | HS_CLR_SRQ_INT |
			HS_CLR_EOI_EMPTY_INT | HS_CLR_HF_INT, nec_priv->iobase + HS_MODE );
		cb_priv->in_fifo_half_full = 0;

		outb( cb_priv->hs_mode_bits & HS_SYS_CONTROL, nec_priv->iobase + HS_MODE );

outb( irq_bits( cb_priv->irq ), nec_priv->iobase + HS_INT_LEVEL );

		cb_priv->hs_mode_bits &= ~HS_ENABLE_MASK;
		cb_priv->hs_mode_bits |= HS_RX_ENABLE;
		outb( cb_priv->hs_mode_bits, nec_priv->iobase + HS_MODE );

clear_bit( READ_READY_BN, &nec_priv->state );
		nec7210_set_reg_bits( nec_priv, IMR2, HR_DMAI, 1 );
udelay(10);
	}else
	{
		nec7210_set_reg_bits( nec_priv, IMR2, HR_DMAI, 0 );
udelay(10);

		cb_priv->hs_mode_bits &= ~HS_ENABLE_MASK;
		outb( cb_priv->hs_mode_bits, nec_priv->iobase + HS_MODE );

		cb7210_internal_interrupt( board );

//		clear_bit( READ_READY_BN, &nec_priv->state );
		if( ( cb7210_line_status( board ) & BusNRFD ) == 0 )
		{
printk("fixing read ready\n");
			clear_bit( READ_READY_BN, &nec_priv->state );
		}
	}

	spin_unlock_irqrestore( &board->spinlock, flags );
}

int read_input_fifo( cb7210_private_t *cb_priv, uint8_t *buffer, unsigned int num_bytes )
{
	nec7210_private_t *nec_priv = &cb_priv->nec7210_priv;
	unsigned long iobase = nec_priv->iobase;
	int count;

	if( num_bytes % cb7210_fifo_width )
	{
		printk( "cb7210: bug! tried to read odd number of bytes in read_input_fifo()\n" );
		return 0;
	}

	count = 0;
	while( count < num_bytes )
	{
		uint16_t word;

		word = inw( iobase + DIR );
		buffer[ count++ ] = word & 0xff;
		buffer[ count++ ] = ( word >> 8 ) & 0xff;
	}

	return count;
}

static ssize_t fifo_read( gpib_board_t *board, cb7210_private_t *cb_priv, uint8_t *buffer, size_t length )
{
	size_t count = 0;
	ssize_t retval = 0;
	nec7210_private_t *nec_priv = &cb_priv->nec7210_priv;
	unsigned long iobase = nec_priv->iobase;
	int hs_status;
	uint16_t word;
	unsigned long flags;
	int have_half_word = 0;

	if( length <= cb7210_fifo_size )
	{
		printk("cb7210: bug! fifo_read() with length < fifo size\n" );
		return -EINVAL;
	}

//input_fifo_enable( board, 1 );
//nec7210_set_reg_bits( nec_priv, IMR2, HR_DMAI, 1 );
	while( count + cb7210_fifo_size <= length )
	{
//input_fifo_enable( board, 0 );
//udelay(10);
//buffer[count++]='&';
//input_fifo_enable( board, 1 );
//udelay(10);
nec7210_set_reg_bits( nec_priv, IMR2, HR_DMAI, 1 );

		if( wait_event_interruptible( board->wait,
			( cb_priv->in_fifo_half_full && have_fifo_word( cb_priv ) ) ||
			test_bit( RECEIVED_END_BN, &nec_priv->state ) ||
			test_bit( TIMO_NUM, &board->status ) ) )
		{
			printk("cb7210: fifo half full wait interrupted\n");
			retval = -ERESTARTSYS;
//			nec7210_set_reg_bits( nec_priv, IMR2, HR_DMAI, 0 );
			break;
		}
		if( test_bit( TIMO_NUM, &board->status ) )
		{
			retval = -ETIMEDOUT;
//			nec7210_set_reg_bits( nec_priv, IMR2, HR_DMAI, 0 );
			break;
		}

		spin_lock_irqsave( &board->spinlock, flags );
#if 0
if( test_bit( RECEIVED_END_BN, &nec_priv->state ) == 0 )
{
	nec_priv->auxa_bits &= ~HR_HANDSHAKE_MASK;
	nec_priv->auxa_bits |= HR_HLDA;
	write_byte( nec_priv, nec_priv->auxa_bits, AUXMR );
}
#endif
nec7210_set_reg_bits( nec_priv, IMR2, HR_DMAI, 0 );
printk("hs 0x%x, cnt %i\n", inb( nec_priv->iobase + HS_STATUS ), count );

		while( ( ( hs_status = inb( nec_priv->iobase + HS_STATUS ) ) &
			( HS_RX_MSB_NOT_EMPTY | HS_RX_LSB_NOT_EMPTY ) ) ==
			( HS_RX_MSB_NOT_EMPTY | HS_RX_LSB_NOT_EMPTY ) )
		{
			word = inw( iobase + DIR );
			if( have_half_word )
			{
				buffer[ count++ ] = ( word >> 8 ) & 0xff;
				buffer[ count++ ] = word & 0xff;
			}else
			{
				buffer[ count++ ] = word & 0xff;
				buffer[ count++ ] = ( word >> 8 ) & 0xff;
			}
		}
if( ( hs_status = inb( nec_priv->iobase + HS_STATUS ) ) &
		( HS_RX_MSB_NOT_EMPTY | HS_RX_LSB_NOT_EMPTY ) )
{
udelay(10);
	word = inw( iobase + DIR );
	if( hs_status & HS_RX_LSB_NOT_EMPTY )
	{
		buffer[ count++ ] = word & 0xff;
		have_half_word = 1;
	}
	if( hs_status & HS_RX_MSB_NOT_EMPTY )
	{
		buffer[ count++ ] = ( word >> 8 ) & 0xff;
		have_half_word = 0;
	}
printk("dreg hs 0x%x\n", hs_status );
//buffer[ count++ ] = 0x82;
}
		cb_priv->in_fifo_half_full = 0;

		spin_unlock_irqrestore( &board->spinlock, flags );
		if( test_bit( RECEIVED_END_BN, &nec_priv->state ) ||
			( hs_status & HS_FIFO_FULL ) ||
( count + cb7210_fifo_size >= length ) )
		{
printk("end or ff or count hs 0x%x\n", hs_status );
			break;
		}
#if 0
	nec_priv->auxa_bits &= ~HR_HANDSHAKE_MASK;
	nec_priv->auxa_bits |= HR_HLDE;
	write_byte( nec_priv, nec_priv->auxa_bits, AUXMR );
	udelay( 10 );
	write_byte( nec_priv, AUX_FH, AUXMR);
#endif
	}
	if( test_bit( RECEIVED_END_BN, &nec_priv->state ) == 0 )
	{
//		nec_priv->auxa_bits &= ~HR_HANDSHAKE_MASK;
//		nec_priv->auxa_bits |= HR_HLDA;
//		write_byte( nec_priv, nec_priv->auxa_bits, AUXMR );
//XXX
//		udelay(10);

	}

#if 0
	nec_priv->auxa_bits &= ~HR_HANDSHAKE_MASK;
	nec_priv->auxa_bits |= HR_HLDA;
	write_byte( nec_priv, nec_priv->auxa_bits, AUXMR );
#endif

nec7210_set_reg_bits( nec_priv, IMR2, HR_DMAI, 0 );
//udelay(100);
	while( ( hs_status = inb( nec_priv->iobase + HS_STATUS ) ) &
			( HS_RX_MSB_NOT_EMPTY | HS_RX_LSB_NOT_EMPTY ) )
	{
		word = inw( iobase + DIR );
		if( have_half_word )
		{
if( hs_status & HS_RX_MSB_NOT_EMPTY )
			buffer[ count++ ] = ( word >> 8 ) & 0xff;
if( hs_status & HS_RX_LSB_NOT_EMPTY )
			buffer[ count++ ] = word & 0xff;
		}else
		{
if( hs_status & HS_RX_LSB_NOT_EMPTY )
			buffer[ count++ ] = word & 0xff;
if( hs_status & HS_RX_MSB_NOT_EMPTY )
			buffer[ count++ ] = ( word >> 8 ) & 0xff;
		}
	}

	return retval ? retval : count;
}

ssize_t cb7210_accel_read( gpib_board_t *board, uint8_t *buffer,
	size_t length, int *end )
{
	ssize_t retval, count = 0;
	cb7210_private_t *cb_priv = board->private_data;
	nec7210_private_t *nec_priv = &cb_priv->nec7210_priv;
	unsigned long flags;

	//XXX deal with lack of eos capability when using fifos
	if( length <= cb7210_fifo_size /* || ( nec_priv->auxa_bits & HR_REOS XXX ) */ )
{
	nec7210_set_auxa_bits( nec_priv, HR_HANDSHAKE_MASK, 0 );
	nec7210_set_auxa_bits( nec_priv, HR_HLDE, 1 );
		return cb7210_read( board, buffer, length, end );
}
	*end = 0;

	/* release rfd holdoff */
	nec7210_set_auxa_bits( nec_priv, HR_HANDSHAKE_MASK, 0 );
	nec7210_set_auxa_bits( nec_priv, HR_HLDE, 1 );
	write_byte( nec_priv, AUX_FH, AUXMR );
	nec7210_set_auxa_bits( nec_priv, HR_HANDSHAKE_MASK, 0 );
	nec7210_set_auxa_bits( nec_priv, HR_HLDE, 1 );

	if( wait_event_interruptible( board->wait,
		test_bit( READ_READY_BN, &nec_priv->state ) ||
		test_bit( TIMO_NUM, &board->status ) ) )
	{
		printk("cb7210: read ready wait interrupted\n");
		return -ERESTARTSYS;
	}
	if( test_bit( TIMO_NUM, &board->status ) )
		return -ETIMEDOUT;

	input_fifo_enable( board, 1 );

	count = fifo_read( board, cb_priv, buffer, length - 1 );
	input_fifo_enable( board, 0 );
	if( count < 0 )
		return count;

	if( test_and_clear_bit( RECEIVED_END_BN, &nec_priv->state ) )
	{
		*end = 1;
		return count;
	}
#if 0
	retval = cb7210_read( board, &buffer[ count ], 1, end );
	if( retval < 0 ) return retval;
	count += retval;
#endif

	nec7210_set_auxa_bits( nec_priv, HR_HANDSHAKE_MASK, 0 );
	nec7210_set_auxa_bits( nec_priv, HR_HLDA, 1 );

	if( wait_event_interruptible( board->wait,
		test_bit( READ_READY_BN, &nec_priv->state ) ||
		test_bit( TIMO_NUM, &board->status ) ) )
	{
		printk("cb7210: second read ready wait interrupted\n");
		return -ERESTARTSYS;
	}
	if( test_bit( TIMO_NUM, &board->status ) )
		return -ETIMEDOUT;

	spin_lock_irqsave( &board->spinlock, flags );
	buffer[ count++ ] = read_byte( nec_priv, DIR );
	clear_bit( READ_READY_BN, &nec_priv->state );
	spin_unlock_irqrestore( &board->spinlock, flags );

	return count;
}






