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


ssize_t agilent_82350b_accel_write( gpib_board_t *board, uint8_t *buffer, size_t length, int send_eoi )
{
	agilent_82350b_private_t *a_priv = board->private_data;
	tms9914_private_t *tms_priv = &a_priv->tms9914_priv;
	int i, j;
	unsigned short event_status;
	ssize_t retval = 0;
	int fifoTransferLength = length;
	if(send_eoi)
	{
		--fifoTransferLength;
	}
	read_and_clear_event_status(board);
	for(i = 0; i < fifoTransferLength;)
	{
		int block_size;
		int complement;
		if(fifoTransferLength - i < agilent_82350b_fifo_size)
			block_size = fifoTransferLength - i;
		else
			block_size = agilent_82350b_fifo_size;
		writeb(0, a_priv->gpib_base + SRAM_ACCESS_CONTROL_REG); 
		complement = -block_size;
		writeb(complement & 0xff, a_priv->gpib_base + XFER_COUNT_LO_REG);
		writeb((complement >> 8) & 0xff, a_priv->gpib_base + XFER_COUNT_MID_REG);
		writeb((complement >> 16) & 0xf, a_priv->gpib_base + XFER_COUNT_HI_REG);
		for(j = 0; j < block_size; ++j, ++i)
		{
			// load data into board's sram
			writeb(buffer[i], a_priv->sram_base + j); 
		}
		writeb(ENABLE_TI_TO_SRAM, a_priv->gpib_base + SRAM_ACCESS_CONTROL_REG); 
		if(readb(a_priv->gpib_base + STREAM_STATUS_REG) & HALTED_STATUS_BIT) 
			writeb(RESTART_STREAM_BIT, a_priv->gpib_base + STREAM_STATUS_REG); 
		if(wait_event_interruptible(board->wait, 
			((event_status = read_and_clear_event_status(board)) & TERM_COUNT_STATUS_BIT) ||
			test_bit(DEV_CLEAR_BN, &tms_priv->state) ||
			test_bit(TIMO_NUM, &board->status)))

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
	writeb(0, a_priv->gpib_base + SRAM_ACCESS_CONTROL_REG); 
	if(retval) return retval;
	if(send_eoi)
	{
		retval = agilent_82350b_write(board, buffer + fifoTransferLength, 1, 1);
		if(retval < 0) return retval;
	}
	return length;
}



