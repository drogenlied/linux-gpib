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

	// enable command out interrupt
	imr2_bits |= HR_COIE;
	GPIBout(IMR2, imr2_bits);

	while(count < length)
	{
		if(wait_event_interruptible(nec7210_wait, test_and_clear_bit(0, &command_out_ready)))
		{
			printk("gpib command wait interrupted\n");
			break;
		}
		GPIBout(CDOR, buffer[count]);
		count++;
	}

	// disable command out interrupt
	imr2_bits |= HR_COIE;
	GPIBout(IMR2, imr2_bits);

	return count;
}









