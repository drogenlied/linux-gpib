/***************************************************************************
                          agilent_82350b/agilent_82350b_write.c  -  description
                             -------------------

    copyright            : (C) 2002, 2004 by Frank Mori Hess
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

#include "agilent_82350b.h"


int agilent_82350b_accel_write( gpib_board_t *board, uint8_t *buffer, size_t length, int send_eoi, size_t *bytes_written)
{
	agilent_82350b_private_t *a_priv = board->private_data;
	tms9914_private_t *tms_priv = &a_priv->tms9914_priv;
	int i, j;
	unsigned short event_status;
	int retval = 0;
	int fifoTransferLength = length;
	int block_size = 0;
	*bytes_written = 0;
	if(send_eoi)
	{
		--fifoTransferLength;
	}
	clear_bit(DEV_CLEAR_BN, &tms_priv->state);
	read_and_clear_event_status(board);
	writeb(0, a_priv->gpib_base + SRAM_ACCESS_CONTROL_REG); 
	for(i = 0; i < fifoTransferLength;)
	{
		if(fifoTransferLength - i < agilent_82350b_fifo_size)
			block_size = fifoTransferLength - i;
		else
			block_size = agilent_82350b_fifo_size;
		set_transfer_counter(a_priv, block_size);
		for(j = 0; j < block_size; ++j, ++i)
		{
			// load data into board's sram
			writeb(buffer[i], a_priv->sram_base + j); 
		}
		clear_bit(WRITE_READY_BN, &tms_priv->state);
		writeb(ENABLE_TI_TO_SRAM, a_priv->gpib_base + SRAM_ACCESS_CONTROL_REG); 
		if(readb(a_priv->gpib_base + STREAM_STATUS_REG) & HALTED_STATUS_BIT) 
			writeb(RESTART_STREAM_BIT, a_priv->gpib_base + STREAM_STATUS_REG); 
		retval = wait_event_interruptible(board->wait,
			((event_status = read_and_clear_event_status(board)) & TERM_COUNT_STATUS_BIT) ||
			test_bit(DEV_CLEAR_BN, &tms_priv->state) ||
			test_bit(TIMO_NUM, &board->status));
		writeb(0, a_priv->gpib_base + SRAM_ACCESS_CONTROL_REG); 
		*bytes_written += block_size - read_transfer_counter(a_priv);
		if(retval)
		{
			printk("%s: write wait interrupted\n", __FILE__);
			retval = -ERESTARTSYS;
			break;
		}
		if(test_bit(TIMO_NUM, &board->status))
		{
			printk("%s: minor %i: write timed out\n", __FILE__, board->minor);
			retval = -ETIMEDOUT;
			break;
		}
		if(test_bit(DEV_CLEAR_BN, &tms_priv->state))
		{
			printk("%s: device clear interrupted write\n", __FILE__);
			retval = -EINTR;
			break;
		}
	}
	if(retval) return retval;
	if(send_eoi)
	{
		size_t num_bytes;
		retval = agilent_82350b_write(board, buffer + fifoTransferLength, 1, 1, &num_bytes);
		*bytes_written += num_bytes;
		if(retval < 0) return retval;
	}
	return 0;
}



