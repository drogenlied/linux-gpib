/***************************************************************************
                              tms9914/read.c
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
#include <linux/spinlock.h>

void check_for_eos( tms9914_private_t *priv, uint8_t byte )
{
	if( ( priv->eos_flags & REOS ) == 0 ) return;

	if( priv->eos_flags & BIN )
	{
		if( priv->eos == byte )
			set_bit( RECEIVED_END_BN, &priv->state );
	}else
	{
		if( ( priv->eos & 0x7f ) == ( byte & 0x7f ) )
			set_bit( RECEIVED_END_BN, &priv->state );
	}
}

int need_release_holdoff(gpib_board_t *board, tms9914_private_t *priv)
{
	unsigned long flags;
	int retval;
	unsigned int line_status = tms9914_line_status(board, priv);
	if((line_status & BusNRFD) == 0 || (line_status & BusDAV))
		return 0;
	spin_lock_irqsave(&board->spinlock, flags);
	tms9914_interrupt(board, priv);
	if(test_bit(READ_READY_BN, &priv->state))
		retval = 0;
	else
		retval = 1;
	spin_unlock_irqrestore(&board->spinlock, flags);
	return retval;
}

static ssize_t pio_read(gpib_board_t *board, tms9914_private_t *priv, uint8_t *buffer, size_t length, int *nbytes)
{
	ssize_t retval = 0;
	unsigned long flags;

	*nbytes = 0;
	while(*nbytes < length)
	{
		if(need_release_holdoff(board, priv))
			write_byte( priv, AUX_RHDF, AUXCR );
		if(wait_event_interruptible(board->wait,
			test_bit( READ_READY_BN, &priv->state ) ||
			test_bit( DEV_CLEAR_BN, &priv->state ) ||
			test_bit( TIMO_NUM, &board->status ) ) )
		{
			printk("gpib: pio read wait interrupted\n");
			retval = -ERESTARTSYS;
			break;
		};
		if( test_bit( TIMO_NUM, &board->status ) )
		{
			retval = -ETIMEDOUT;
			break;
		}
		if( test_bit( DEV_CLEAR_BN, &priv->state ) )
		{
			retval = -EINTR;
			break;
		}

		spin_lock_irqsave( &board->spinlock, flags );
		clear_bit( READ_READY_BN, &priv->state );
		buffer[ (*nbytes)++ ] = read_byte( priv, DIR );
		spin_unlock_irqrestore( &board->spinlock, flags );

		check_for_eos( priv, buffer[ *nbytes - 1 ] );

		if( test_bit( RECEIVED_END_BN, &priv->state ) )
			break;
	}

	return retval;
}

ssize_t tms9914_read(gpib_board_t *board, tms9914_private_t *priv, uint8_t *buffer, size_t length, int *end, int *nbytes)
{
	ssize_t retval = 0;
	int bytes_read;
	
	*end = 0;
	*nbytes = 0;
	if(length == 0) return 0;

	clear_bit( DEV_CLEAR_BN, &priv->state );

	if( priv->eos_flags & REOS )
	{
		write_byte(priv, AUX_HLDA | AUX_CS, AUXCR);
		write_byte(priv, AUX_HLDE, AUXCR);
	}else
	{
		write_byte(priv, AUX_HLDA, AUXCR);
		write_byte(priv, AUX_HLDE | AUX_CS, AUXCR);
	}
	// transfer data (except for last byte)
	length--;
	if(length)
	{
		// PIO transfer
		retval = pio_read(board, priv, buffer, length, &bytes_read);
		*nbytes += bytes_read;
		if(retval < 0)
			return retval;
	}

	// read last byte if we havn't received an END yet
	if(test_bit(RECEIVED_END_BN, &priv->state) == 0)
	{
		// make sure we holdoff after last byte read
		write_byte(priv, AUX_HLDE, AUXCR);
		write_byte(priv, AUX_HLDA | AUX_CS, AUXCR);
		retval = pio_read(board, priv, &buffer[*nbytes], 1, &bytes_read);
		*nbytes += bytes_read;
		if(retval < 0)
			return retval;
	}

	if(test_and_clear_bit(RECEIVED_END_BN, &priv->state))
		*end = 1;

	return 0;
}

EXPORT_SYMBOL(tms9914_read);






