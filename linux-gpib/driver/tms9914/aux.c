/***************************************************************************
                                 tms9914/aux.c
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

int tms9914_take_control(gpib_board_t *board, tms9914_private_t *priv, int syncronous)
{
	int i;
	const int timeout = 1000;

	if(syncronous)
	{
		write_byte(priv, AUX_TCS, AUXCR);
	}else
		write_byte(priv, AUX_TCA, AUXCR);
	// busy wait until ATN is asserted
	for(i = 0; i < timeout; i++)
	{
		if((read_byte(priv, ADSR) & HR_ATN))
			break;
		udelay(1);
	}

	tms9914_update_status( board, priv );

	if( i == timeout )
		return -ETIMEDOUT;

	return 0;
}

int tms9914_go_to_standby(gpib_board_t *board, tms9914_private_t *priv)
{
	int i;
	const int timeout = 1000;

	write_byte(priv, AUX_GTS, AUXCR);
	// busy wait until ATN is released
	for(i = 0; i < timeout; i++)
	{
		if((read_byte(priv, ADSR) & HR_ATN) == 0)
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

void tms9914_interface_clear(gpib_board_t *board, tms9914_private_t *priv, int assert)
{
	if(assert)
	{
		write_byte(priv, AUX_SIC | AUX_CS, AUXCR);
		set_bit(CIC_NUM, &board->status);
	}else
		write_byte(priv, AUX_SIC, AUXCR);
}

void tms9914_remote_enable(gpib_board_t *board, tms9914_private_t *priv, int enable)
{
	if(enable)
		write_byte(priv, AUX_SRE | AUX_CS, AUXCR);
	else
		write_byte(priv, AUX_SRE, AUXCR);
}

EXPORT_SYMBOL(tms9914_take_control);
EXPORT_SYMBOL(tms9914_go_to_standby);
EXPORT_SYMBOL(tms9914_interface_clear);
EXPORT_SYMBOL(tms9914_remote_enable);

