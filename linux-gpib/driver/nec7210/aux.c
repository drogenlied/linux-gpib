/***************************************************************************
                                 nec7210/aux.c
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
#include <asm/bitops.h>

int nec7210_take_control(gpib_device_t *device, nec7210_private_t *priv, int syncronous)
{
	int i;
	const int timeout = 1000;
	int retval = 0;
	unsigned int adsr_bits = 0;

	if(syncronous)
	{
		// make sure we aren't asserting rfd holdoff
		if(test_and_clear_bit(RFD_HOLDOFF_BN, &priv->state))
		{
			write_byte(priv, AUX_FH, AUXMR);
		}
		write_byte(priv, AUX_TCS, AUXMR);
	}else
		write_byte(priv, AUX_TCA, AUXMR);
	// busy wait until ATN is asserted
	for(i = 0; i < timeout; i++)
	{
		adsr_bits = read_byte(priv, ADSR);
		if((adsr_bits & HR_NATN) == 0)
		{
			break;
		}
		udelay(1);
	}
	// suspend if we still don't have ATN
	if(i == timeout)
	{
		while(((adsr_bits = read_byte(priv, ADSR)) & HR_NATN) &&
			test_bit(TIMO_NUM, &device->status) == 0)
		{
			if(interruptible_sleep_on_timeout(&device->wait, 1))
			{
				printk("interupted waiting for ATN\n");
				retval = -EINTR;
				break;
			}
		}
	}

	if(test_bit(TIMO_NUM, &device->status))
	{
		printk("gpib: take control timed out\n");
		retval = -ETIMEDOUT;
	}
	if(adsr_bits & HR_NATN)
		clear_bit(ATN_NUM, &device->status);
	else
		set_bit(ATN_NUM, &device->status);

	return retval;
}

int nec7210_go_to_standby(gpib_device_t *device, nec7210_private_t *priv)
{
	int i;
	const int timeout = 1000;
	unsigned int adsr_bits = 0;
	int retval = 0;

	write_byte(priv, AUX_GTS, AUXMR);
	// busy wait until ATN is released
	for(i = 0; i < timeout; i++)
	{
		adsr_bits = read_byte(priv, ADSR);
		if(adsr_bits & HR_NATN)
			break;
		udelay(1);
	}
	if(i == timeout)
	{
		printk("error waiting for NATN\n");
		retval = -ETIMEDOUT;
	}
	if(adsr_bits & HR_NATN)
		clear_bit(ATN_NUM, &device->status);
	else
		set_bit(ATN_NUM, &device->status);

	return retval;
}

void nec7210_interface_clear(gpib_device_t *device, nec7210_private_t *priv, int assert)
{
	if(assert)
		write_byte(priv, AUX_SIFC, AUXMR);
	else
		write_byte(priv, AUX_CIFC, AUXMR);
}

void nec7210_remote_enable(gpib_device_t *device, nec7210_private_t *priv, int enable)
{
	if(enable)
		write_byte(priv, AUX_SREN, AUXMR);
	else
		write_byte(priv, AUX_CREN, AUXMR);
}

EXPORT_SYMBOL(nec7210_take_control);
EXPORT_SYMBOL(nec7210_go_to_standby);
EXPORT_SYMBOL(nec7210_interface_clear);
EXPORT_SYMBOL(nec7210_remote_enable);

