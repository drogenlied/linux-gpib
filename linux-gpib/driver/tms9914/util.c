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

void tms9914_enable_eos(gpib_board_t *board, tms9914_private_t *priv, uint8_t eos_byte, int compare_8_bits)
{
	priv->eos = eos_byte;
	priv->eos_flags = REOS;
	if( compare_8_bits )
		priv->eos_flags |= BIN;
}

void tms9914_disable_eos(gpib_board_t *board, tms9914_private_t *priv)
{
	priv->eos_flags &= ~REOS;
}

int tms9914_parallel_poll(gpib_board_t *board, tms9914_private_t *priv, uint8_t *result)
{
	int ret;

	// execute parallel poll
	write_byte(priv, AUX_RPP, AUXCR);

	// wait for result
	ret = wait_event_interruptible(board->wait, test_bit(COMMAND_READY_BN, &priv->state));

	if(ret)
	{
		printk("gpib: parallel poll interrupted\n");
		return -EINTR;
	}

	*result = read_byte(priv, CPTR);

	return 0;
}

void tms9914_parallel_poll_configure( gpib_board_t *board,
	tms9914_private_t *priv, uint8_t config )
{
	uint8_t dio_byte;

	if( config & PPC_DISABLE )
		dio_byte = 0;
	else
		dio_byte = 1 << ( 7 - ( config & PPC_DIO_MASK ) );

	if( config & PPC_SENSE )
	{
		// XXX assert line on ist
	}

	dio_byte = 0;
	
	write_byte( priv, dio_byte, PPR );
}

void tms9914_serial_poll_response(gpib_board_t *board, tms9914_private_t *priv, uint8_t status)
{
	unsigned long flags;
	
	spin_lock_irqsave( &board->spinlock, flags );
	write_byte(priv, status, SPMR);
	priv->spoll_status = status;
	spin_unlock_irqrestore( &board->spinlock, flags );
}

uint8_t tms9914_serial_poll_status( gpib_board_t *board, tms9914_private_t *priv )
{
	uint8_t status;
	unsigned long flags;

	spin_lock_irqsave( &board->spinlock, flags );
	status = priv->spoll_status;
	spin_unlock_irqrestore( &board->spinlock, flags );

	return status;
}

void tms9914_primary_address(gpib_board_t *board, tms9914_private_t *priv, unsigned int address)
{
	// put primary address in address0
	write_byte(priv, address & ADDRESS_MASK, ADR);
}

void tms9914_secondary_address(gpib_board_t *board, tms9914_private_t *priv, unsigned int address, int enable)
{
	if( enable )
		priv->imr1_bits |= HR_APTIE;
	else
		priv->imr1_bits &= ~HR_APTIE;

	write_byte( priv, priv->imr1_bits, IMR1 );
}

unsigned int tms9914_update_status(gpib_board_t *board, tms9914_private_t *priv)
{
	unsigned long flags;
	unsigned int retval;

	spin_lock_irqsave( &board->spinlock, flags );
	retval = update_status_nolock( board, priv );
	spin_unlock_irqrestore( &board->spinlock, flags );

	return retval;
}

unsigned int update_status_nolock( gpib_board_t *board, tms9914_private_t *priv )
{
	int address_status;

	address_status = read_byte( priv, ADSR );

	// check for remote/local
	if(address_status & HR_REM)
		set_bit( REM_NUM, &board->status );
	else
		clear_bit( REM_NUM, &board->status );
	// check for lockout
	if(address_status & HR_LLO)
		set_bit( LOK_NUM, &board->status );
	else
		clear_bit( LOK_NUM, &board->status );
	// check for ATN
	if(address_status & HR_ATN)
	{
		set_bit( ATN_NUM, &board->status );
		clear_bit( WRITE_READY_BN, &priv->state );
	}else
	{
		clear_bit( ATN_NUM, &board->status );
		clear_bit( COMMAND_READY_BN, &priv->state );
	}
	// check for talker/listener addressed
	if(address_status & HR_TA)
	{
		clear_bit( READ_READY_BN, &priv->state );
		set_bit( TACS_NUM, &board->status );
	}else
		clear_bit( TACS_NUM, &board->status );
	if(address_status & HR_LA)
	{
		clear_bit( WRITE_READY_BN, &priv->state );
		set_bit(LACS_NUM, &board->status);
	}else
		clear_bit( LACS_NUM, &board->status );

//	GPIB_DPRINTK( "status 0x%x, state 0x%x\n", board->status, priv->state );
	
	return board->status;
}

int tms9914_line_status( const gpib_board_t *board, tms9914_private_t *priv)
{
	int bsr_bits;
	int status = ValidALL;

	bsr_bits = read_byte(priv, BSR);
	
	if( bsr_bits & BSR_REN_BIT )
		status |= BusREN;
	if( bsr_bits & BSR_IFC_BIT )
		status |= BusIFC;
	if( bsr_bits & BSR_SRQ_BIT )
		status |= BusSRQ;
	if( bsr_bits & BSR_EOI_BIT )
		status |= BusEOI;
	if( bsr_bits & BSR_NRFD_BIT )
		status |= BusNRFD;
	if( bsr_bits & BSR_NDAC_BIT )
		status |= BusNDAC;
	if( bsr_bits & BSR_DAV_BIT )
		status |= BusDAV;
	if( bsr_bits & BSR_ATN_BIT )
		status |= BusATN;

	return status;
}

EXPORT_SYMBOL(tms9914_enable_eos);
EXPORT_SYMBOL(tms9914_disable_eos);
EXPORT_SYMBOL(tms9914_serial_poll_response);
EXPORT_SYMBOL(tms9914_serial_poll_status);
EXPORT_SYMBOL(tms9914_parallel_poll);
EXPORT_SYMBOL(tms9914_parallel_poll_configure);
EXPORT_SYMBOL(tms9914_primary_address);
EXPORT_SYMBOL(tms9914_secondary_address);
EXPORT_SYMBOL(tms9914_update_status);
EXPORT_SYMBOL(tms9914_line_status);

