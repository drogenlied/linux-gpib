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

void nec7210_enable_eos(gpib_board_t *board, nec7210_private_t *priv, uint8_t eos_byte, int compare_8_bits)
{
	write_byte(priv, eos_byte, EOSR);
	priv->auxa_bits |= HR_REOS;
	if(compare_8_bits)
		priv->auxa_bits |= HR_BIN;
	else
		priv->auxa_bits &= ~HR_BIN;
	write_byte(priv, priv->auxa_bits, AUXMR);
}

void nec7210_disable_eos(gpib_board_t *board, nec7210_private_t *priv)
{
	priv->auxa_bits &= ~HR_REOS;
	write_byte(priv, priv->auxa_bits, AUXMR);
}

int nec7210_parallel_poll(gpib_board_t *board, nec7210_private_t *priv, uint8_t *result)
{
	int ret;

	// execute parallel poll
	write_byte(priv, AUX_EPP, AUXMR);

	// wait for result
	ret = wait_event_interruptible(board->wait, test_bit(COMMAND_READY_BN, &priv->state));
	if(ret)
	{
		printk("gpib: parallel poll interrupted\n");
		return -ERESTARTSYS;
	}

	*result = read_byte(priv, CPTR);

	return 0;
}

void nec7210_parallel_poll_response( gpib_board_t *board,
	nec7210_private_t *priv, unsigned int configuration )
{
	write_byte( priv, PPR | configuration , AUXMR );
}

void nec7210_serial_poll_response(gpib_board_t *board, nec7210_private_t *priv, uint8_t status)
{
//	write_byte(priv, 0, SPMR);		/* clear current serial poll status */
	// XXX check pend bit before writing?
	write_byte(priv, status, SPMR);		/* set new status to v */
}

uint8_t nec7210_serial_poll_status( gpib_board_t *board, nec7210_private_t *priv )
{
	return read_byte( priv, SPSR );
}

void nec7210_primary_address(const gpib_board_t *board, nec7210_private_t *priv, unsigned int address)
{
	// put primary address in address0
	write_byte(priv, address & ADDRESS_MASK, ADR);
}

void nec7210_secondary_address(const gpib_board_t *board, nec7210_private_t *priv, unsigned int address, int enable)
{
	if(enable)
	{
		// put secondary address in address1
		write_byte(priv, HR_ARS | (address & ADDRESS_MASK), ADR);
		// go to address mode 2
		priv->reg_bits[ ADMR ] &= ~HR_ADM0;
		priv->reg_bits[ ADMR ] |= HR_ADM1;
	}else
	{
		// disable address1 register
		write_byte(priv, HR_ARS | HR_DT | HR_DL, ADR);
		// go to address mode 1
		priv->reg_bits[ ADMR ] |= HR_ADM0;
		priv->reg_bits[ ADMR ] &= ~HR_ADM1;
	}
	write_byte( priv, priv->reg_bits[ ADMR ], ADMR );
}

unsigned int update_status_nolock( gpib_board_t *board, nec7210_private_t *priv )
{
	int address_status_bits;

	if(priv == NULL) return 0;

	address_status_bits = read_byte(priv, ADSR);

	if(address_status_bits & HR_CIC)
		set_bit(CIC_NUM, &board->status);
	else
		clear_bit(CIC_NUM, &board->status);
	// check for talker/listener addressed
	if(address_status_bits & HR_TA)
	{
		clear_bit(READ_READY_BN, &priv->state);
		set_bit(TACS_NUM, &board->status);
	}else
		clear_bit(TACS_NUM, &board->status);
	if(address_status_bits & HR_LA)
	{
		clear_bit(WRITE_READY_BN, &priv->state);
		set_bit(LACS_NUM, &board->status);
	}else
		clear_bit(LACS_NUM, &board->status);
	if(address_status_bits & HR_NATN)
	{
		clear_bit(COMMAND_READY_BN, &priv->state);
		clear_bit(ATN_NUM, &board->status);
	}else
	{
		clear_bit(WRITE_READY_BN, &priv->state);
		set_bit(ATN_NUM, &board->status);
	}

//	GPIB_DPRINTK( "status 0x%x, state 0x%x\n", board->status, priv->state );

	/* we rely on the interrupt handler to set the
	 * rest of the status bits */

	return board->status;
}

unsigned int nec7210_update_status(gpib_board_t *board, nec7210_private_t *priv)
{
	unsigned long flags;
	unsigned int retval;

	spin_lock_irqsave( &board->spinlock, flags );
	retval = update_status_nolock( board, priv );
	spin_unlock_irqrestore( &board->spinlock, flags );

	return retval;
}

unsigned int nec7210_set_reg_bits( nec7210_private_t *priv, unsigned int reg,
	unsigned int mask, unsigned int bits )
{
	priv->reg_bits[ reg ] &= ~mask;
	priv->reg_bits[ reg ] |= mask & bits;
	write_byte( priv, priv->reg_bits[ reg ], reg );
	return priv->reg_bits[ reg ];
}

void nec7210_set_handshake_mode( gpib_board_t *board, nec7210_private_t *priv, int mode )
{
	unsigned long flags;

	spin_lock_irqsave( &board->spinlock, flags );
	priv->auxa_bits &= ~HR_HANDSHAKE_MASK;
	priv->auxa_bits |= ( mode & HR_HANDSHAKE_MASK );
	write_byte( priv, priv->auxa_bits, AUXMR );
	spin_unlock_irqrestore( &board->spinlock, flags );
}

uint8_t nec7210_read_data_in( gpib_board_t *board, nec7210_private_t *priv, int *end )
{
	unsigned long flags;
	uint8_t data;

	spin_lock_irqsave( &board->spinlock, flags );
	data = read_byte( priv, DIR );
	clear_bit( READ_READY_BN, &priv->state );
	if( test_and_clear_bit( RECEIVED_END_BN, &priv->state ) )
		*end = 1;
	else
		*end = 0;
	spin_unlock_irqrestore( &board->spinlock, flags );

	return data;
}

EXPORT_SYMBOL( nec7210_enable_eos );
EXPORT_SYMBOL( nec7210_disable_eos );
EXPORT_SYMBOL( nec7210_serial_poll_response );
EXPORT_SYMBOL( nec7210_serial_poll_status );
EXPORT_SYMBOL( nec7210_parallel_poll_response );
EXPORT_SYMBOL( nec7210_parallel_poll );
EXPORT_SYMBOL( nec7210_primary_address );
EXPORT_SYMBOL( nec7210_secondary_address );
EXPORT_SYMBOL( nec7210_update_status );
EXPORT_SYMBOL( nec7210_set_reg_bits );
EXPORT_SYMBOL( nec7210_set_handshake_mode );
EXPORT_SYMBOL( nec7210_read_data_in );

