/***************************************************************************
                              nec7210/util.c
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

void nec7210_enable_eos(gpib_driver_t *driver, uint8_t eos_byte, int compare_8_bits)
{
	nec7210_private_t *priv = driver->private_data;

	priv->write_byte(priv, eos_byte, EOSR);
	priv->auxa_bits |= HR_REOS;
	if(compare_8_bits)
		priv->auxa_bits |= HR_BIN;
	else
		priv->auxa_bits &= ~HR_BIN;
	priv->write_byte(priv, priv->auxa_bits, AUXMR);
}

void nec7210_disable_eos(gpib_driver_t *driver)
{
	nec7210_private_t *priv = driver->private_data;

	priv->auxa_bits &= ~HR_REOS;
	priv->write_byte(priv, priv->auxa_bits, AUXMR);
}

int nec7210_parallel_poll(gpib_driver_t *driver, uint8_t *result)
{
	int ret;
	nec7210_private_t *priv = driver->private_data;

	// enable command out interrupts
	priv->imr2_bits |= HR_COIE;
	priv->write_byte(priv, priv->imr2_bits, IMR2);

	// execute parallel poll
	priv->write_byte(priv, AUX_EPP, AUXMR);

	// wait for result
	ret = wait_event_interruptible(driver->wait, test_bit(COMMAND_READY_BN, &priv->state));

	// disable command out interrupts
	priv->imr2_bits &= ~HR_COIE;
	priv->write_byte(priv, priv->imr2_bits, IMR2);

	if(ret)
	{
		printk("gpib: parallel poll interrupted\n");
		return -EINTR;
	}

	*result = priv->read_byte(priv, CPTR);

	return 0;
}

int nec7210_serial_poll_response(gpib_driver_t *driver, uint8_t status)
{
	nec7210_private_t *priv = driver->private_data;

	priv->write_byte(priv, 0, SPMR);		/* clear current serial poll status */
	priv->write_byte(priv, status, SPMR);		/* set new status to v */

	return 0;
}

void nec7210_primary_address(gpib_driver_t *driver, unsigned int address)
{
	nec7210_private_t *priv = driver->private_data;

	// put primary address in address0
	priv->write_byte(priv, address & ADDRESS_MASK, ADR);
}

void nec7210_secondary_address(gpib_driver_t *driver, unsigned int address, int enable)
{
	nec7210_private_t *priv = driver->private_data;

	if(enable)
	{
		// put secondary address in address1
		priv->write_byte(priv, HR_ARS | (address & ADDRESS_MASK), ADR);
		// go to address mode 2
		priv->admr_bits &= ~HR_ADM0;
		priv->admr_bits |= HR_ADM1;
	}else
	{
		// disable address1 register
		priv->write_byte(priv, HR_ARS | HR_DT | HR_DL, ADR);
		// go to address mode 1
		priv->admr_bits |= HR_ADM0;
		priv->admr_bits &= ~HR_ADM1;
	}
	priv->write_byte(priv, priv->admr_bits, ADMR);
}

unsigned int nec7210_update_status(gpib_driver_t *driver)
{
	nec7210_private_t *priv = driver->private_data;
	int address_status_bits;

	if(priv == NULL) return 0;

	address_status_bits = priv->read_byte(priv, ADSR);

	if(address_status_bits & HR_CIC)
		set_bit(CIC_NUM, &driver->status);
	else
		clear_bit(CIC_NUM, &driver->status);
	// check for talker/listener addressed
	if(address_status_bits & HR_TA)
		set_bit(TACS_NUM, &driver->status);
	else
		clear_bit(TACS_NUM, &driver->status);
	if(address_status_bits & HR_LA)
		set_bit(LACS_NUM, &driver->status);
	else
		clear_bit(LACS_NUM, &driver->status);
	if(address_status_bits & HR_NATN)
		clear_bit(ATN_NUM, &driver->status);
	else
		set_bit(ATN_NUM, &driver->status);

	/* we rely on the interrupt handler to set the
	 * bits read from ISR1 and ISR2 */

	return driver->status;
}


