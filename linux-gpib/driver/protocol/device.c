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
 * DVTRG
 * Trigger the device with primary address pad and secondary 
 * address sad.  If the device has no secondary address, pass a
 * zero in for this argument.  This function sends the TAD of the
 * GPIB interface, UNL, the LAD of the device, and GET.
 */ 
IBLCL int dvtrg(int padsad)
{
	uint8_t cmdString[2];
	int status = driver->update_status(driver);

	if((status & CIC) == 0)
	{
		printk("gpib: interface cannot trigger when not CIC\n");
		return -1;
	}
	if(send_setup(padsad))
	{
		return -1;
	}
	cmdString[0] = GET;
	ibcmd(cmdString, 1);
	return 0;
}


/*
 * DVCLR
 * Clear the device with primary address pad and secondary
 * address sad.  If the device has no secondary address, pass a
 * zero in for this argument.  This function sends the TAD of the
 * GPIB interface, UNL, the LAD of the device, and SDC.
 */
IBLCL int dvclr(int padsad)
{
	uint8_t cmdString[2];
	int status = driver->update_status(driver);

	if((status & CIC ) == 0)
	{
		printk("gpib: board not CIC during clear\n");
		return -1;
	}
	if(driver->take_control(driver, 0) < 0)
		return -1;
	if(send_setup(padsad) < 0)
		return -1;
	cmdString[0] = SDC;
	if(driver->command(driver, cmdString, 1) < 0)
		return -1;

	return 0;
}


/*
 * DVRSP
 * This function performs a serial poll of the device with primary
 * address pad and secondary address sad. If the device has no
 * secondary adddress, pass a zero in for this argument.  At the
 * end of a successful serial poll the response is returned in result.
 * SPD and UNT are sent at the completion of the poll.
 */


IBLCL int dvrsp(int padsad, uint8_t *result)
{
	uint8_t cmd_string[8];
	int status = driver->update_status(driver);
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

	osStartTimer(pollTimeidx);

	driver->take_control(driver, 0);

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

	if (driver->command(driver, cmd_string, i) < i)
		return -1;

	driver->go_to_standby(driver);

	// read poll result
	ret = driver->read(driver, result, 1, &end_flag);
	if(ret < 1)
	{
		printk("gpib: serial poll failed\n");
		return -1;
	}

	driver->take_control(driver, 0);

	cmd_string[0] = SPD;	/* disable serial poll bytes */
	cmd_string[1] = UNT;
	if(driver->command(driver, cmd_string, 2) < 2 )
	{
		return -1;
	}
	osRemoveTimer();

	return 0;
}

/*
 * DVWRT
 * Write cnt bytes to the device with primary address pad and
 * secondary address sad.  If the device has no secondary address,
 * pass a zero in for this argument.  The device is adressed to
 * listen and the GPIB interface is addressed to talk.  ibwrt is
 * then called to write cnt bytes to the device from buf.
 */
IBLCL ssize_t dvwrt(int padsad, uint8_t *buf, unsigned int cnt)
{
	int status = driver->update_status(driver);
	if((status & CIC) == 0)
	{
		return -1;
	}
	if (send_setup(padsad) < 0)
		return -1;

	// XXX assumes all the data is written in this call
	return ibwrt(buf, cnt, 0);
}


/*
 * 488.2 Controller sequences
 */
IBLCL int receive_setup(int padsad)
{
	uint8_t pad, sad;
	uint8_t cmdString[8];
	unsigned int i = 0;

	pad = padsad;
	sad = padsad >> 8;
	if ((pad > 0x1E) || (sad && ((sad < 0x60) || (sad > 0x7E)))) {
		printk("gpib bad addr");
		return -1;
	}

	cmdString[i++] = UNL;

	cmdString[i++] = myPAD | LAD;	/* controller's listen address */
	if (mySAD)
		cmdString[i++] = mySAD;
	cmdString[i++] = pad | TAD;
	if (sad)
		cmdString[i++] = sad;

	if (ibcmd(cmdString, i) < 0)
		return -1;

	return 0;
}


IBLCL int send_setup(int padsad)
{
	uint8_t pad, sad;
	uint8_t cmdString[8];
	unsigned i = 0;

	pad = padsad;
	sad = padsad >> 8;
	if ((pad > 0x1E) || (sad && ((sad < 0x60) || (sad > 0x7E)))) {
		printk("gpib: bad addr\n");
		return -1;
	}

	cmdString[i++] = UNL;
	cmdString[i++] = pad | LAD;
	if (sad)
		cmdString[i++] = sad;
	cmdString[i++] = myPAD | TAD;	/* controller's talk address */
	if (mySAD)
		cmdString[i++] = mySAD;

	if (ibcmd(cmdString, i) < 0)
		return -1;


	return 0;
}


