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
#include <linux/wait.h>
#include <asm/bitops.h>

int nec7210_take_control(int syncronous)
{
	if(syncronous)
		GPIBout(AUXMR, AUX_TCS);
	else
		GPIBout(AUXMR, AUX_TCA);
	// wait until we have control
	while(GPIBin(ADSR) & HR_NATN)
	{
		if(interruptible_sleep_on_timeout(&nec7210_status_wait, 1))
		{
			printk("error waiting for ATN\n");
			return -1;
		}
	}
	return 0;
}

int nec7210_go_to_standby(void)
{
	GPIBout(AUXMR, AUX_GTS);
	while((GPIBin(ADSR) & HR_NATN) == 0)
	{
		if(interruptible_sleep_on_timeout(&nec7210_status_wait, 1))
		{
			printk("error waiting for NATN\n");
			return -1;
		}
	}
	return 0;
}

void nec7210_interface_clear(int assert)
{
	if(assert)
		GPIBout(AUXMR, AUX_SIFC);
	else
		GPIBout(AUXMR, AUX_CIFC);
}

void nec7210_remote_enable(int enable)
{
	if(enable)
		GPIBout(AUXMR, AUX_SREN);
	else
		GPIBout(AUXMR, AUX_CREN);
}
