/***************************************************************************
                              protocol/device.c
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

#include <ibprot.h>

/*
 * DVRSP
 * This function performs a serial poll of the device with primary
 * address pad and secondary address sad. If the device has no
 * secondary adddress, pass a zero in for this argument.  At the
 * end of a successful serial poll the response is returned in result.
 * SPD and UNT are sent at the completion of the poll.
 */

int dvrsp(gpib_board_t *board, int padsad, uint8_t *result)
{
	uint8_t cmd_string[8];
	int status = ibstatus(board);
	int end_flag;
	ssize_t ret;
	unsigned int pad, sad;
	int i;

	if((status & CIC) == 0)
	{
		printk("gpib: not CIC during serial poll\n");
		return -1;
	}

	pad = padsad & 0xff;
	sad = (padsad >> 8) & 0xff;
	if ((pad > 0x1E) || (sad && ((sad < 0x60) || (sad > 0x7E)))) {
		printk("gpib: bad address for serial poll");
		return -1;
	}

	osStartTimer(board, pollTimeidx);

	board->interface->take_control(board, 0);

	i = 0;
	cmd_string[i++] = UNL;
	cmd_string[i++] = myPAD | LAD;	/* controller's listen address */
	if (mySAD)
		cmd_string[i++] = mySAD;
	cmd_string[i++] = SPE;	//serial poll enable
	// send talk address
	cmd_string[i++] = pad | TAD;
	if (sad)
		cmd_string[i++] = sad;

	if (board->interface->command(board, cmd_string, i) < i)
	{
		printk("gpib: failed to setup serial poll\n");
		return -1;
	}

	board->interface->go_to_standby(board);

	// read poll result
	ret = board->interface->read(board, result, 1, &end_flag);
	if(ret < 1)
	{
		printk("gpib: serial poll failed\n");
		return -1;
	}

	board->interface->take_control(board, 0);

	cmd_string[0] = SPD;	/* disable serial poll bytes */
	cmd_string[1] = UNT;
	if(board->interface->command(board, cmd_string, 2) < 2 )
	{
		printk("gpib: failed to disable serial poll\n");
		return -1;
	}
	osRemoveTimer(board);

	return 0;
}

