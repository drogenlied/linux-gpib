/***************************************************************************
                              tms9914/util.c
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
#include <linux/delay.h>

void tms9914_enable_eos(gpib_device_t *device, tms9914_private_t *priv, uint8_t eos_byte, int compare_8_bits)
{
	// XXX
}

void tms9914_disable_eos(gpib_device_t *device, tms9914_private_t *priv)
{
	// XXX
}

int tms9914_parallel_poll(gpib_device_t *device, tms9914_private_t *priv, uint8_t *result)
{
	int ret;

	// execute parallel poll
	write_byte(priv, AUX_RPP, AUXCR);

	// wait for result
	ret = wait_event_interruptible(device->wait, test_bit(COMMAND_READY_BN, &priv->state));

	if(ret)
	{
		printk("gpib: parallel poll interrupted\n");
		return -EINTR;
	}

	*result = read_byte(priv, CPTR);

	return 0;
}

int tms9914_serial_poll_response(gpib_device_t *device, tms9914_private_t *priv, uint8_t status)
{
	write_byte(priv, status, SPMR);		/* set new status to v */

	return 0;
}

void tms9914_primary_address(gpib_device_t *device, tms9914_private_t *priv, unsigned int address)
{
	// put primary address in address0
	write_byte(priv, address & ADDRESS_MASK, ADR);
}

void tms9914_secondary_address(gpib_device_t *device, tms9914_private_t *priv, unsigned int address, int enable)
{
	//XXX
}

unsigned int tms9914_update_status(gpib_device_t *device, tms9914_private_t *priv)
{
	int address_status;

	address_status = read_byte(priv, ADSR);

	// check for remote/local
	if(address_status & HR_REM)
		set_bit(REM_NUM, &device->status);
	else
		clear_bit(REM_NUM, &device->status);
	// check for lockout
	if(address_status & HR_LLO)
		set_bit(LOK_NUM, &device->status);
	else
		clear_bit(LOK_NUM, &device->status);
	// check for ATN
	if(address_status & HR_ATN)
		set_bit(ATN_NUM, &device->status);
	else
		clear_bit(ATN_NUM, &device->status);
	// check for talker/listener addressed
	if(address_status & HR_TA)
		set_bit(TACS_NUM, &device->status);
	else
		clear_bit(TACS_NUM, &device->status);
	if(address_status & HR_LA)
		set_bit(LACS_NUM, &device->status);
	else
		clear_bit(LACS_NUM, &device->status);

	return device->status;
}

EXPORT_SYMBOL(tms9914_enable_eos);
EXPORT_SYMBOL(tms9914_disable_eos);
EXPORT_SYMBOL(tms9914_serial_poll_response);
EXPORT_SYMBOL(tms9914_parallel_poll);
EXPORT_SYMBOL(tms9914_primary_address);
EXPORT_SYMBOL(tms9914_secondary_address);
EXPORT_SYMBOL(tms9914_update_status);

