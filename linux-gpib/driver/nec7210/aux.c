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

int nec7210_take_control(gpib_driver_t *driver, int syncronous)
{
	int i;
	const int timeout = 1000;
	nec7210_private_t *priv = driver->private_data;

	if(syncronous)
	{
		// make sure we aren't asserting rfd holdoff
		if(test_and_clear_bit(RFD_HOLDOFF_BN, &priv->state))
		{
			priv->write_byte(priv, AUX_FH, AUXMR);
		}
		priv->write_byte(priv, AUX_TCS, AUXMR);
	}else
		priv->write_byte(priv, AUX_TCA, AUXMR);
	// busy wait until ATN is asserted
	for(i = 0; i < timeout; i++)
	{
		if((priv->read_byte(priv, ADSR) & HR_NATN) == 0)
			break;
		udelay(1);
	}
	// suspend if we still don't have ATN
	if(i == timeout)
	{
		while((priv->read_byte(priv, ADSR) & HR_NATN) &&
			test_bit(TIMO_NUM, &driver->status) == 0)
		{
			if(interruptible_sleep_on_timeout(&driver->wait, 1))
			{
				printk("interupted for ATN\n");
				return -EINTR;
			}
		}
	}

	if(test_bit(TIMO_NUM, &driver->status))
	{
		printk("gpib: take control timed out\n");
		return -ETIMEDOUT;
	}

	return 0;
}

int nec7210_go_to_standby(gpib_driver_t *driver)
{
	int i;
	const int timeout = 1000;
	nec7210_private_t *priv = driver->private_data;

	priv->write_byte(priv, AUX_GTS, AUXMR);
	// busy wait until ATN is released
	for(i = 0; i < timeout; i++)
	{
		if(priv->read_byte(priv, ADSR) & HR_NATN)
			break;
		udelay(1);
	}
	if(i == timeout)
	{
		printk("error waiting for NATN\n");
		return -ETIMEDOUT;
	}
	return 0;
}

void nec7210_interface_clear(gpib_driver_t *driver, int assert)
{
	nec7210_private_t *priv = driver->private_data;

	if(assert)
		priv->write_byte(priv, AUX_SIFC, AUXMR);
	else
		priv->write_byte(priv, AUX_CIFC, AUXMR);
}

void nec7210_remote_enable(gpib_driver_t *driver, int enable)
{
	nec7210_private_t *priv = driver->private_data;
	
	if(enable)
		priv->write_byte(priv, AUX_SREN, AUXMR);
	else
		priv->write_byte(priv, AUX_CREN, AUXMR);
}
