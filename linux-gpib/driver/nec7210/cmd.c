/***************************************************************************
                                    nec7210/cmd.c
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

ssize_t nec7210_command(gpib_board_t *board, nec7210_private_t *priv, uint8_t
 *buffer, size_t length)
{
	size_t count = 0;
	ssize_t retval = 0;
	unsigned long flags;

	while(count < length)
	{
		if(wait_event_interruptible(board->wait, test_bit(COMMAND_READY_BN, &priv->state) ||
			test_bit(TIMO_NUM, &board->status)))
		{
			printk("gpib command wait interrupted\n");
			retval = -EINTR;
			break;
		}
		if(test_bit(TIMO_NUM, &board->status))
		{
			break;
		}

		spin_lock_irqsave(&board->spinlock, flags);
		clear_bit(COMMAND_READY_BN, &priv->state);
		write_byte(priv, buffer[count], CDOR);
		spin_unlock_irqrestore(&board->spinlock, flags);

		count++;
	}
	// wait for last byte to get sent
	if(wait_event_interruptible(board->wait, test_bit(COMMAND_READY_BN, &priv->state) ||
		test_bit(TIMO_NUM, &board->status)))
	{
		printk("gpib command wait interrupted\n");
		retval = -EINTR;
	}
	if(test_bit(TIMO_NUM, &board->status))
	{
		printk("gpib command timed out\n");
		retval = -ETIMEDOUT;
	}

	return retval ? retval : count;
}

EXPORT_SYMBOL(nec7210_command);








