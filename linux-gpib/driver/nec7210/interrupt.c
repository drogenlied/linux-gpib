/***************************************************************************
                              nec7210/interrupt.c
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
void nec7210_interrupt( gpib_board_t *board, nec7210_private_t *priv )
{
	int status1, status2;

	// read interrupt status (also clears status)
	status1 = read_byte(priv, ISR1);
	status2 = read_byte(priv, ISR2);

	nec7210_interrupt_have_status( board, priv, status1, status2 );
}

void nec7210_interrupt_have_status( gpib_board_t *board,
	nec7210_private_t *priv, int status1, int status2 )
{
	unsigned long flags;

	spin_lock(&board->spinlock);

	// record service request in status
	if(status2 & HR_SRQI)
	{
		set_bit(SRQI_NUM, &board->status);
	}

	// change in lockout status
	if(status2 & HR_LOKC)
	{
		if(status2 & HR_LOK)
			set_bit(LOK_NUM, &board->status);
		else
			clear_bit(LOK_NUM, &board->status);
	}

	// change in remote status
	if(status2 & HR_REMC)
	{
		if(status2 & HR_REM)
			set_bit(REM_NUM, &board->status);
		else
			clear_bit(REM_NUM, &board->status);
	}

	// record address status change in status
	if(status2 & HR_ADSC)
	{
		update_status_nolock( board, priv );
	}

	// record reception of END
	if(status1 & HR_END)
	{
		set_bit(RECEIVED_END_BN, &priv->state);
	}

	// get incoming data in PIO mode
	if((status1 & HR_DI))
	{
		set_bit(READ_READY_BN, &priv->state);
	}

	// check for dma read transfer complete
	if(test_bit(DMA_READ_IN_PROGRESS_BN, &priv->state))
	{
		flags = claim_dma_lock();
		disable_dma(priv->dma_channel);
		clear_dma_ff(priv->dma_channel);
		if((status1 & HR_END) || get_dma_residue(priv->dma_channel) == 0)
		{
			clear_bit(DMA_READ_IN_PROGRESS_BN, &priv->state);
		}else
			enable_dma(priv->dma_channel);
		release_dma_lock(flags);
	}

	if((status1 & HR_DO))
	{
		set_bit(WRITE_READY_BN, &priv->state);
		if(test_bit(DMA_WRITE_IN_PROGRESS_BN, &priv->state))	// write data, isa dma mode
		{
			// check if dma transfer is complete
			flags = claim_dma_lock();
			disable_dma(priv->dma_channel);
			clear_dma_ff(priv->dma_channel);
			if(get_dma_residue(priv->dma_channel) == 0)
			{
				clear_bit(DMA_WRITE_IN_PROGRESS_BN, &priv->state);
			}else
			{
				clear_bit(WRITE_READY_BN, &priv->state);
				enable_dma(priv->dma_channel);
			}
			release_dma_lock(flags);
		}else
		{
		}
	}

	// outgoing command can be sent
	if(status2 & HR_CO)
	{
		set_bit(COMMAND_READY_BN, &priv->state);
	}

	// command pass through received
	if(status1 & HR_CPT)
	{
		unsigned int command;

		command = read_byte(priv, CPTR);
		write_byte(priv, AUX_NVAL, AUXMR);
		printk( "gpib: command pass through 0x%x\n", command );
	}

	if(status1 & HR_ERR)
	{
		set_bit( BUS_ERROR_BN, &priv->state );
		GPIB_DPRINTK( "gpib: bus error\n" );
	}

	if( status1 & HR_DEC )
	{
		push_gpib_event( &board->event_queue, EventDevClr );
		// XXX should clear buffers, etc.
	}

	if( status1 & HR_DET )
	{
		push_gpib_event( &board->event_queue, EventDevTrg );
	}

	spin_unlock(&board->spinlock);

	if( ( status1 & priv->imr1_bits ) || ( status2 & priv->imr2_bits ) )
	{
		GPIB_DPRINTK( "isr1 0x%x, imr1 0x%x, isr2 0x%x, imr2 0x%x, status 0x%x\n",
			status1, priv->imr1_bits, status2, priv->imr2_bits, board->status);
		wake_up_interruptible( &board->wait ); /* wake up sleeping process */
	}
}

EXPORT_SYMBOL(nec7210_interrupt);
EXPORT_SYMBOL(nec7210_interrupt_have_status);


