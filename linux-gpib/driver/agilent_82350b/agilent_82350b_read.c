/***************************************************************************
                          agilent_82350b/agilent_82350b_read.c  -  description
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

ssize_t agilent_82350b_accel_read( gpib_board_t *board, uint8_t *buffer, size_t length, int *end, int *nbytes)
{
	agilent_82350b_private_t *a_priv = board->private_data;
	tms9914_private_t *tms_priv = &a_priv->tms9914_priv;
	int retval = 0;
	unsigned short event_status;
	int need_release;
	//hardware doesn't support checking for end-of-string character when using fifo
	if(tms_priv->eos_flags & REOS) 
	{
		return tms9914_read( board, tms_priv, buffer, length, end, nbytes);
	}
	clear_bit( DEV_CLEAR_BN, &tms_priv->state );
	read_and_clear_event_status(board);
	*end = 0;
	*nbytes = 0;
	if(length == 0) return 0;
	--length;
	//disable fifo for the moment
	writeb(DIRECTION_GPIB_TO_HOST, a_priv->gpib_base + SRAM_ACCESS_CONTROL_REG);
	// handle corner case of board not in holdoff and one byte has slipped in already
	need_release = tms9914_need_release_holdoff(board, tms_priv);
	if(!need_release)
	{
		int bytes_read;
		retval = tms9914_read(board, tms_priv, buffer, 1, end, &bytes_read);
		*nbytes += bytes_read;
		if(retval < 0 || *end)
		{
			printk("%s: retval=%i end=%i\n", __FUNCTION__, retval, *end);
			return retval;
		}
	}
	write_byte(tms_priv, AUX_HLDE | AUX_CS, AUXCR);
	write_byte(tms_priv, AUX_HLDA, AUXCR);
	write_byte(tms_priv, AUX_RHDF, AUXCR);
	while(*nbytes < length && *end == 0)
	{
		int block_size;
		int j;
		int count;
		if(length - *nbytes < agilent_82350b_fifo_size)
			block_size = length - *nbytes;
		else
			block_size = agilent_82350b_fifo_size;
		set_transfer_counter(a_priv, block_size);
		writeb(ENABLE_TI_TO_SRAM | DIRECTION_GPIB_TO_HOST, a_priv->gpib_base + SRAM_ACCESS_CONTROL_REG); 
		if(readb(a_priv->gpib_base + STREAM_STATUS_REG) & HALTED_STATUS_BIT) 
			writeb(RESTART_STREAM_BIT, a_priv->gpib_base + STREAM_STATUS_REG); 
		clear_bit(READ_READY_BN, &tms_priv->state);
		if(wait_event_interruptible(board->wait, 
			((event_status = read_and_clear_event_status(board)) & (TERM_COUNT_STATUS_BIT | BUFFER_END_STATUS_BIT)) ||
			test_bit(DEV_CLEAR_BN, &tms_priv->state) ||
			test_bit(TIMO_NUM, &board->status)))
		{
			printk("%s: write wait interrupted\n", __FILE__);
			retval = -ERESTARTSYS;
			break;
		}
		count = block_size - read_transfer_counter(a_priv);
		for(j = 0; j < count && *nbytes < length; ++j)
			buffer[(*nbytes)++] = readb(a_priv->sram_base + j); 
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
		if(event_status & BUFFER_END_STATUS_BIT)
		{
			clear_bit(RECEIVED_END_BN, &tms_priv->state);
			*end = 1;
		}
	}
	write_byte(tms_priv, AUX_HLDA | AUX_CS, AUXCR);
	write_byte(tms_priv, AUX_HLDE, AUXCR);
	writeb(DIRECTION_GPIB_TO_HOST, a_priv->gpib_base + SRAM_ACCESS_CONTROL_REG);
	if(retval < 0) return retval;
	// read last byte if we havn't received an END yet
	if(*end == 0)
	{
		int bytes_read;
		// try to make sure we holdoff after last byte read
		retval = tms9914_read(board, tms_priv, &buffer[*nbytes], 1, end, &bytes_read);
		*nbytes += bytes_read;
		if(retval < 0)
			return retval;
	}
	return 0;
}
