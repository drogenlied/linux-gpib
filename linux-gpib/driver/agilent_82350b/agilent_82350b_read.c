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
	int i;
	int complement;
	int retval = 0;
	unsigned short event_status;
	//hardware doesn't support checking for end-of-string character when using fifo
	if(tms_priv->eos_flags & REOS) 
	{
		return tms9914_read( board, tms_priv, buffer, length, end, nbytes);
	}
printk("%s: using fifo\n", __FUNCTION__);
	clear_bit( DEV_CLEAR_BN, &tms_priv->state );
	read_and_clear_event_status(board);
	*end = 0;
	*nbytes = 0;
	if(length == 0) return 0;
	--length;
	write_byte(tms_priv, AUX_HLDA, AUXCR);
	write_byte(tms_priv, AUX_HLDE | AUX_CS, AUXCR);
	writeb(DIRECTION_GPIB_TO_HOST, a_priv->gpib_base + SRAM_ACCESS_CONTROL_REG);
	complement = -length;
	writeb(complement & 0xff, a_priv->gpib_base + XFER_COUNT_LO_REG);
	writeb((complement >> 8) & 0xff, a_priv->gpib_base + XFER_COUNT_MID_REG);
	writeb((complement >> 16) & 0xf, a_priv->gpib_base + XFER_COUNT_HI_REG);
	writeb(ENABLE_TI_TO_SRAM | DIRECTION_GPIB_TO_HOST, a_priv->gpib_base + SRAM_ACCESS_CONTROL_REG); 
	for(i = 0; i < length;)
	{
		if(readb(a_priv->gpib_base + STREAM_STATUS_REG) & HALTED_STATUS_BIT) 
			writeb(RESTART_STREAM_BIT, a_priv->gpib_base + STREAM_STATUS_REG); 
		if(wait_event_interruptible(board->wait, 
			((event_status = read_and_clear_event_status(board)) & (TERM_COUNT_STATUS_BIT | BUFFER_END_STATUS_BIT)) ||
			test_bit(DEV_CLEAR_BN, &tms_priv->state) ||
			test_bit(TIMO_NUM, &board->status)))
		{
			printk("%s: write wait interrupted\n", __FILE__);
			retval = -ERESTARTSYS;
			break;
		}
		printk("Count regs: lo=0x%x mid=0x%x hi=0x%x\n",
			readb(a_priv->gpib_base + XFER_COUNT_LO_REG),
			readb(a_priv->gpib_base + XFER_COUNT_MID_REG),
			readb(a_priv->gpib_base + XFER_COUNT_HI_REG));
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
		if(event_status & (BUFFER_END_STATUS_BIT | TERM_COUNT_STATUS_BIT))
		{
			int j;
			for(j = 0; j < agilent_82350b_fifo_size && i < length; ++j)
				buffer[i++] = readb(a_priv->sram_base + j); 
		}
		if(test_bit(RECEIVED_END_BN, &tms_priv->state) == 0)
			break;
	}
	if(test_and_clear_bit(RECEIVED_END_BN, &tms_priv->state))
		*end = 1;
	//FIXME try to empty fifo before returning if we were interrupted
	*nbytes = i;
	if(retval < 0) return retval;
	// read last byte if we havn't received an END yet
	if(test_bit(RECEIVED_END_BN, &tms_priv->state) == 0)
	{
		int bytes_read;
		// make sure we holdoff after last byte read
		write_byte(tms_priv, AUX_HLDE, AUXCR);
		write_byte(tms_priv, AUX_HLDA | AUX_CS, AUXCR);
		retval = tms9914_read(board, tms_priv, &buffer[*nbytes], 1, end, &bytes_read);
		*nbytes += bytes_read;
		if(retval < 0)
			return retval;
	}
	return 0;
}
