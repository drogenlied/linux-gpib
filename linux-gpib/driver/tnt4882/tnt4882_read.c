/***************************************************************************
                          tnt4882_read.c  -  description
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

#include "tnt4882.h"
#include <linux/delay.h>

static int fifo_word_available( tnt4882_private_t *tnt_priv )
{
	int status2;
	int retval;

	status2 = tnt_readb( tnt_priv, STS2 );
	retval = ( status2 & AEFN ) && ( status2 & BEFN );

	return retval;
}

static int fifo_byte_available( tnt4882_private_t *tnt_priv )
{
	int status2;
	int retval;

	status2 = tnt_readb( tnt_priv, STS2 );
	retval = ( status2 & AEFN ) || ( status2 & BEFN );

	return retval;
}

static int fifo_xfer_done( tnt4882_private_t *tnt_priv )
{
	int status1;
	int retval;

	status1 = tnt_readb( tnt_priv, STS1 );
	retval = status1 & ( S_DONE | S_HALT );

	return retval;
}

static int drain_fifo_words(tnt4882_private_t *tnt_priv, uint8_t *buffer, int num_bytes)
{
	int count = 0;
	nec7210_private_t *nec_priv = &tnt_priv->nec7210_priv;

	while(fifo_word_available( tnt_priv ) && count + 2 <= num_bytes)
	{
		short word;

		word = tnt_priv->io_readw( nec_priv->iobase + FIFOB );
		buffer[ count++ ] = word & 0xff;
		buffer[ count++ ] = ( word >> 8 ) & 0xff;
	}
	return count;
}

ssize_t tnt4882_accel_read( gpib_board_t *board, uint8_t *buffer, size_t length, int *end )
{
	size_t count = 0;
	ssize_t retval = 0;
	tnt4882_private_t *tnt_priv = board->private_data;
	nec7210_private_t *nec_priv = &tnt_priv->nec7210_priv;
	unsigned int bits, imr1_bits, imr2_bits;
	int32_t hw_count;
	unsigned long flags;

	imr1_bits = nec_priv->reg_bits[ IMR1 ];
	imr2_bits = nec_priv->reg_bits[ IMR2 ];
	nec7210_set_reg_bits( nec_priv, IMR1, 0xff, HR_ENDIE | HR_DECIE );
	if( tnt_priv->chipset != TNT4882 )
		nec7210_set_reg_bits( nec_priv, IMR2, 0xff, HR_DMAI );
	else
		nec7210_set_reg_bits( nec_priv, IMR2, 0xff, 0 );

	tnt_writeb( tnt_priv, RESET_FIFO, CMDR );
	udelay(1);

	tnt_writeb( tnt_priv, nec_priv->auxa_bits | HR_HLDA, CCR );
	bits = TNT_TLCHE | TNT_B_16BIT | TNT_IN | TNT_CCEN;
	tnt_writeb( tnt_priv, bits, CFG );

	// load 2's complement of count into hardware counters
	hw_count = -length;
	tnt_writeb( tnt_priv, hw_count & 0xff, CNT0 );
	tnt_writeb( tnt_priv, ( hw_count >> 8 ) & 0xff, CNT1 );
	tnt_writeb( tnt_priv, ( hw_count >> 16 ) & 0xff, CNT2 );
	tnt_writeb( tnt_priv, ( hw_count >> 24 ) & 0xff, CNT3 );

	spin_lock_irqsave( &board->spinlock, flags );
	tnt_priv->imr3_bits |= HR_DONE | HR_NEF;
	tnt_writeb( tnt_priv, tnt_priv->imr3_bits, IMR3 );
	spin_unlock_irqrestore( &board->spinlock, flags );

	nec7210_set_handshake_mode( board, nec_priv, HR_HLDA );
	write_byte( nec_priv, AUX_FH, AUXMR );
	nec7210_set_handshake_mode( board, nec_priv, HR_HLDE );

	tnt_writeb( tnt_priv, GO, CMDR );
	udelay(1);

	while(count + 1 < length &&
		test_bit( RECEIVED_END_BN, &nec_priv->state ) == 0)
	{
		// wait until byte is ready
		if( wait_event_interruptible( board->wait,
			fifo_word_available( tnt_priv ) ||
			fifo_xfer_done( tnt_priv ) ||
			test_bit( RECEIVED_END_BN, &nec_priv->state ) ||
			test_bit( DEV_CLEAR_BN, &nec_priv->state ) ||
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
		if( test_bit( DEV_CLEAR_BN, &nec_priv->state ) )
		{
			retval = -EINTR;
			break;
		}

		spin_lock_irqsave( &board->spinlock, flags );
		count += drain_fifo_words(tnt_priv, &buffer[count], length - count);
		tnt_priv->imr3_bits |= HR_NEF;
		tnt_writeb( tnt_priv, tnt_priv->imr3_bits, IMR3 );
		spin_unlock_irqrestore( &board->spinlock, flags );

		if( current->need_resched )
			schedule();
	}
	// wait for last byte
	if( count < length )
	{
		if( wait_event_interruptible( board->wait,
			fifo_byte_available( tnt_priv ) ||
			fifo_xfer_done( tnt_priv ) ||
			test_bit( RECEIVED_END_BN, &nec_priv->state ) ||
			test_bit( DEV_CLEAR_BN, &nec_priv->state ) ||
			test_bit( TIMO_NUM, &board->status ) ) )
		{
			printk("gpib write interrupted\n");
			retval = -ERESTARTSYS;
		}
		if( test_bit( TIMO_NUM, &board->status ) )
		{
			retval = -ETIMEDOUT;
		}
		if( test_bit( DEV_CLEAR_BN, &nec_priv->state ) )
		{
			retval = -EINTR;
		}
		count += drain_fifo_words(tnt_priv, &buffer[count], length - count);
		if(fifo_byte_available( tnt_priv ) && count < length)
		{
			buffer[ count++ ] = tnt_readb( tnt_priv, FIFOB );
		}
	}
	if( test_and_clear_bit( RECEIVED_END_BN, &nec_priv->state ) )
	{
		*end = 1;
	}

	tnt_writeb( tnt_priv, STOP, CMDR );
	udelay(1);

	nec7210_set_reg_bits( nec_priv, IMR1, 0xff, imr1_bits );
	nec7210_set_reg_bits( nec_priv, IMR2, 0xff, imr2_bits );

	if( retval < 0 )
	{
		// force immediate holdoff
		write_byte( nec_priv, AUX_HLDI, AUXMR );
		set_bit( RFD_HOLDOFF_BN, &nec_priv->state );
		return retval;
	}

	return count;
}












