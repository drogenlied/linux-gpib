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

// XXX
IBLCL ssize_t nec7210_command(gpib_driver_t *driver, uint8_t *buffer, size_t length)
{
	size_t count = 0;
	const int timeout = 1000;
	int i;
	nec7210_private_t *priv = driver->private_data;

	// enable command out interrupt
	priv->imr2_bits |= HR_COIE;
	priv->write_byte(priv, priv->imr2_bits, IMR2);

	while(count < length && test_bit(TIMO_NUM, &driver->status) == 0)
	{
		// try a busy wait first before we suspend
		for(i = 0; i < timeout; i++)
		{
			if(test_and_clear_bit(COMMAND_READY_BN, &priv->state))
				break;
		}
		if(i == timeout)
		{
			if(wait_event_interruptible(driver->wait, test_and_clear_bit(COMMAND_READY_BN, &priv->state) ||
				test_bit(TIMO_NUM, &driver->status)))
			{
				printk("gpib command wait interrupted\n");
				break;
			}
		}
		if(test_bit(TIMO_NUM, &driver->status) == 0)
		{
			priv->write_byte(priv, buffer[count], CDOR);
			count++;
		}
	}

	// disable command out interrupt
	priv->imr2_bits |= HR_COIE;
	priv->write_byte(priv, priv->imr2_bits, IMR2);

	return count ? count : -1;
}









