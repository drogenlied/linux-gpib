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

	if(syncronous)
	{
		// make sure we aren't asserting rfd holdoff
		if(pgmstat & PS_HELD);
		{
			GPIBout(AUXMR, AUX_FH);
			pgmstat &= ~PS_HELD;
		}
		GPIBout(AUXMR, AUX_TCS);
	}else
		GPIBout(AUXMR, AUX_TCA);
	// busy wait until ATN is asserted
	for(i = 0; i < timeout; i++)
	{
		if((GPIBin(ADSR) & HR_NATN) == 0)
			break;
		udelay(1);
	}
	// suspend if we still don't have ATN
	if(i == timeout)
	{
		while(GPIBin(ADSR) & HR_NATN )
		{
			if(interruptible_sleep_on_timeout(&nec7210_wait, 1))
			{
				printk("error waiting for ATN\n");
				return -1;
			}
		}
	}

	return 0;
}

int nec7210_go_to_standby(gpib_driver_t *driver)
{
	int i;
	const int timeout = 1000;

	GPIBout(AUXMR, AUX_GTS);
	// busy wait until ATN is released
	for(i = 0; i < timeout; i++)
	{
		if(GPIBin(ADSR) & HR_NATN)
			break;
		udelay(1);
	}
	if(i == timeout)
	{
		printk("error waiting for NATN\n");
		return -1;
	}
	return 0;
}

void nec7210_interface_clear(gpib_driver_t *driver, int assert)
{
	if(assert)
		GPIBout(AUXMR, AUX_SIFC);
	else
		GPIBout(AUXMR, AUX_CIFC);
}

void nec7210_remote_enable(gpib_driver_t *driver, int enable)
{
	if(enable)
		GPIBout(AUXMR, AUX_SREN);
	else
		GPIBout(AUXMR, AUX_CREN);
}
