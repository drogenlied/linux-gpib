/***************************************************************************
                              tms9914/write.c
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

static ssize_t pio_write(gpib_board_t *board, tms9914_private_t *priv, uint8_t *buffer, size_t length)
{
	size_t count = 0;
	ssize_t retval = 0;
	unsigned long flags;

	while(count < length)
	{
		// wait until byte is ready to be sent
		if(wait_event_interruptible(board->wait, test_bit(WRITE_READY_BN, &priv->state) ||
			test_bit(TIMO_NUM, &board->status)))
		{
			printk("gpib write interrupted!\n");
			retval = -EINTR;
			break;
		}
		if(test_bit(TIMO_NUM, &board->status))
		{
			break;
		}

		spin_lock_irqsave(&board->spinlock, flags);
		clear_bit(WRITE_READY_BN, &priv->state);
		write_byte(priv, buffer[count++], CDOR);
		spin_unlock_irqrestore(&board->lock, flags);
	}
	// wait till last byte gets sent
	if(wait_event_interruptible(board->wait, test_bit(WRITE_READY_BN, &priv->state) ||
		test_bit(TIMO_NUM, &board->status)))
	{
		printk("gpib write interrupted!\n");
		retval = -EINTR;
	}
	if(test_bit(TIMO_NUM, &board->status))
	{
		retval = -ETIMEDOUT;
	}

	if(retval)
		return retval;

	return length;
}


ssize_t tms9914_write(gpib_board_t *board, tms9914_private_t *priv, uint8_t *buffer, size_t length, int send_eoi)
{
	size_t count = 0;
	ssize_t retval = 0;

	if(length == 0) return 0;

	//make active talker
	write_byte(priv, AUX_TON | AUX_CS, AUXCR);

	if(send_eoi)
	{
		length-- ; /* save the last byte for sending EOI */
	}

	if(length > 0)
	{
		// PIO transfer
		retval = pio_write(board, priv, buffer, length);
		if(retval < 0)
			return retval;
		else count += retval;
	}
	if(send_eoi)
	{
		/*send EOI */
		write_byte(priv, AUX_SEOI, AUXCR);

		retval = pio_write(board, priv, &buffer[count], 1);
		if(retval < 0)
			return retval;
		else
			count++;
	}

	return count ? count : -1;
}

EXPORT_SYMBOL(tms9914_write);











