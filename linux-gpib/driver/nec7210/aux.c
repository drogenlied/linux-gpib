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

int nec7210_take_control(gpib_board_t *board, nec7210_private_t *priv, int syncronous)
{
	int i;
	const int timeout = 1000;
	int retval = 0;
	unsigned int adsr_bits = 0;

	if(syncronous)
	{
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
	if( i == timeout )
		retval = -ETIMEDOUT;

	return retval;
}

int nec7210_go_to_standby(gpib_board_t *board, nec7210_private_t *priv)
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

	return retval;
}

void nec7210_request_system_control( gpib_board_t *board, nec7210_private_t *priv,
	int request_control )
{
	if( request_control == 0 )
	{
		write_byte( priv, AUX_DSC, AUXMR );
	}
}

void nec7210_interface_clear(gpib_board_t *board, nec7210_private_t *priv, int assert)
{
	if(assert)
		write_byte(priv, AUX_SIFC, AUXMR);
	else
		write_byte(priv, AUX_CIFC, AUXMR);
}

void nec7210_remote_enable(gpib_board_t *board, nec7210_private_t *priv, int enable)
{
	if(enable)
		write_byte(priv, AUX_SREN, AUXMR);
	else
		write_byte(priv, AUX_CREN, AUXMR);
}

EXPORT_SYMBOL( nec7210_request_system_control );
EXPORT_SYMBOL( nec7210_take_control );
EXPORT_SYMBOL( nec7210_go_to_standby );
EXPORT_SYMBOL( nec7210_interface_clear );
EXPORT_SYMBOL( nec7210_remote_enable );

