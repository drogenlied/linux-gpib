/***************************************************************************
                              tms9914/interrupt.c
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
#include <asm/bitops.h>
#include <asm/dma.h>

/*
 *  interrupt service routine
 */

void tms9914_interrupt(gpib_board_t *board, tms9914_private_t *priv)
{
	int status0, status1, address_status;

	spin_lock(&board->spinlock);

	// read interrupt status (also clears status)
	status0 = read_byte(priv, ISR0);
	status1 = read_byte(priv, ISR1);

	// record service request in status
	if(status1 & HR_SRQ)
	{
		set_bit(SRQI_NUM, &board->status);
		wake_up_interruptible(&board->wait);
	}
	
	// have been addressed
	if(status1 & HR_MA)
	{
		printk("addressed!\n");
		// clear dac holdoff
		write_byte(priv, AUX_DHDF, AUXCR);
	}

	// record address status change
	if(status0 & HR_RLC || status0 & HR_MAC)
	{
		address_status = read_byte(priv, ADSR);
		// check for remote/local
		if(address_status & HR_REM)
			set_bit(REM_NUM, &board->status);
		else
			clear_bit(REM_NUM, &board->status);
		// check for lockout
		if(address_status & HR_LLO)
			set_bit(LOK_NUM, &board->status);
		else
			clear_bit(LOK_NUM, &board->status);
		// check for ATN
		if(address_status & HR_ATN)
			set_bit(ATN_NUM, &board->status);
		else
			clear_bit(ATN_NUM, &board->status);
		// check for talker/listener addressed
		if(address_status & HR_TA)
			set_bit(TACS_NUM, &board->status);
		else
			clear_bit(TACS_NUM, &board->status);
		if(address_status & HR_LA)
			set_bit(LACS_NUM, &board->status);
		else
			clear_bit(LACS_NUM, &board->status);
		wake_up_interruptible(&board->wait); /* wake up sleeping process */
	}

	// record reception of END
	if(status0 & HR_END)
	{
		set_bit(RECEIVED_END_BN, &priv->state);
	}

	// get incoming data in PIO mode
	if((status0 & HR_BI))
	{
		set_bit(READ_READY_BN, &priv->state);
		wake_up_interruptible(&board->wait); /* wake up sleeping process */
	}

	if((status0 & HR_BO))
	{
		if(read_byte(priv, ADSR) & HR_ATN)
		{
			set_bit(COMMAND_READY_BN, &priv->state);
		}else
		{
			set_bit(WRITE_READY_BN, &priv->state);
		}
		wake_up_interruptible(&board->wait);
	}

	// unrecognized command received
	if(status1 & HR_UNC)
	{
		printk("gpib command pass thru 0x%x\n", read_byte(priv, CPTR));
	}

	if(status1 & HR_ERR)
	{
		printk("gpib error detected\n");
	}

	spin_unlock(&board->spinlock);

printk("isr0 0x%x, imr0 0x%x, isr1 0x%x, imr1 0x%x, status 0x%x\n", status0, priv->imr0_bits, status1, priv->imr1_bits, board->status);
}

EXPORT_SYMBOL(tms9914_interrupt);


