/***************************************************************************
                          tnt4882_write.c  -  description
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

static int fifo_word_available( gpib_board_t *board,
	tnt4882_private_t *tnt_priv )
{
	unsigned long iobase = tnt_priv->nec7210_priv.iobase;
	int status2;
	int retval;
	unsigned long flags;

	status2 = tnt_priv->io_read( iobase + STS2 );
	retval = ( status2 & AFFN ) && ( status2 & BFFN );

	return retval;
}

static int fifo_xfer_done( gpib_board_t *board,
	tnt4882_private_t *tnt_priv )
{
	unsigned long iobase = tnt_priv->nec7210_priv.iobase;
	int status1;
	int retval;
	unsigned long flags;

	status1 = tnt_priv->io_read( iobase + STS1 );
	retval = status1 & S_DONE;

	return retval;
}

ssize_t tnt4882_accel_write( gpib_board_t *board, uint8_t *buffer, size_t length, int send_eoi )
{
	size_t count = 0;
	ssize_t retval = 0;
	tnt4882_private_t *tnt_priv = board->private_data;
	nec7210_private_t *nec_priv = &tnt_priv->nec7210_priv;
	unsigned long iobase = nec_priv->iobase;
	unsigned int bits, imr1_bits, imr2_bits;
	int32_t hw_count;
	unsigned long flags;

	imr1_bits = nec_priv->reg_bits[ IMR1 ];
	imr2_bits = nec_priv->reg_bits[ IMR2 ];
	nec7210_set_reg_bits( nec_priv, IMR1, 0xff, 0 );
	nec7210_set_reg_bits( nec_priv, IMR1, HR_ERRIE | HR_DECIE, 1 );
	nec7210_set_reg_bits( nec_priv, IMR2, 0xff, 0 );
	nec7210_set_reg_bits( nec_priv, IMR2, HR_DMAO, 1 );

	tnt_priv->io_write( RESET_FIFO, iobase + CMDR );
	udelay(1);

	bits = TNT_TLCHE | TNT_B_16BIT;
	if( send_eoi )
	{
		bits |= TNT_CCEN;
		tnt_priv->io_write( AUX_SEOI, iobase + CCR );
	}
	tnt_priv->io_write( bits, iobase + CFG );

	// load 2's complement of count into hardware counters
	hw_count = -length;
	tnt_priv->io_write( hw_count & 0xff, iobase + CNT0 );
	tnt_priv->io_write( ( hw_count >> 8 ) & 0xff, iobase + CNT1 );
	tnt_priv->io_write( ( hw_count >> 16 ) & 0xff, iobase + CNT2 );
	tnt_priv->io_write( ( hw_count >> 24 ) & 0xff, iobase + CNT3 );

	tnt_priv->io_write( GO, iobase + CMDR );
	udelay(1);

	spin_lock_irqsave( &board->spinlock, flags );
	tnt_priv->imr3_bits |= HR_DONE;
	tnt_priv->io_write( tnt_priv->imr3_bits, iobase + IMR3 );
	spin_unlock_irqrestore( &board->spinlock, flags );

	while( count < length  )
	{
		// wait until byte is ready to be sent
		if( wait_event_interruptible( board->wait,
			fifo_word_available( board, tnt_priv ) ||
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
		spin_lock_irqsave( &board->spinlock, flags );
		while( fifo_word_available( board, tnt_priv ) && count < length )
		{
			uint16_t word;

			word = buffer[ count++ ] & 0xff;
			if( count < length )
				word |= ( buffer[ count++ ] << 8 ) & 0xff00;
			//XXX
			writew( word, iobase + FIFOB );
		}
		tnt_priv->imr3_bits |= HR_NFF;
		tnt_priv->io_write( tnt_priv->imr3_bits, iobase + IMR3 );
		spin_unlock_irqrestore( &board->spinlock, flags );

	}
	// wait last byte has been sent
	if( wait_event_interruptible( board->wait,
		fifo_xfer_done( board, tnt_priv ) ||
		test_bit( TIMO_NUM, &board->status ) ) )
	{
		printk("gpib write interrupted\n");
		retval = -ERESTARTSYS;
	}
	if( test_bit( TIMO_NUM, &board->status ) )
	{
		retval = -ETIMEDOUT;
	}

	tnt_priv->io_write( STOP, iobase + CMDR );
	udelay(1);

	nec7210_set_reg_bits( nec_priv, IMR1, 0xff, 0 );
	nec7210_set_reg_bits( nec_priv, IMR1, imr1_bits, 1 );
	nec7210_set_reg_bits( nec_priv, IMR2, 0xff, 0 );
	nec7210_set_reg_bits( nec_priv, IMR2, imr2_bits, 1 );

	if( retval < 0 )
		return retval;

	return length;
}












