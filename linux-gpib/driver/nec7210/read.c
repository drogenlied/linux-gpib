/***************************************************************************
                              nec7210/read.c
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
#include <asm/dma.h>

ssize_t pio_read(gpib_driver_t *driver, nec7210_private_t *priv, uint8_t *buffer, size_t length)
{
	size_t count = 0;
	ssize_t retval = 0;

	// enable 'data in' and 'end' interrupt
	priv->imr1_bits |= HR_DIIE | HR_ENDIE;
	priv->write_byte(priv, priv->imr1_bits, IMR1);


	while(count < length)
	{
		// try busy wait XXX
		if(wait_event_interruptible(driver->wait,
			test_bit(READ_READY_BN, &priv->state) ||
			test_bit(TIMO_NUM, &driver->status)))
		{
			printk("gpib: pio read wait interrupted\n");
			retval = -EINTR;
			break;
		};
		if(test_bit(TIMO_NUM, &driver->status))
			break;

		clear_bit(READ_READY_BN, &priv->state);
		buffer[count++] = priv->read_byte(priv, DIR);
	}

	// disable 'data in' and 'end' interrupt
	priv->imr1_bits &= ~HR_DIIE & ~HR_ENDIE;
	priv->write_byte(priv, priv->imr1_bits, IMR1);

	return retval ? retval : count;
}

ssize_t dma_read(gpib_driver_t *driver, nec7210_private_t *priv, uint8_t *buffer, size_t length)
{
	ssize_t retval = 0;
	size_t count = 0;
	unsigned long flags;

	flags = claim_dma_lock();
	disable_dma(priv->dma_channel);

	/* program dma controller */
	clear_dma_ff(priv->dma_channel);
	set_dma_count(priv->dma_channel, length);
	set_dma_addr (priv->dma_channel, virt_to_bus(buffer));
	set_dma_mode(priv->dma_channel, DMA_MODE_READ);
	release_dma_lock(flags);

	enable_dma(priv->dma_channel);

	set_bit(DMA_IN_PROGRESS_BN, &priv->state);

	// enable 'data in' and 'end' interrupt
	priv->imr1_bits |= HR_DIIE | HR_ENDIE;
	priv->write_byte(priv, priv->imr1_bits, IMR1);

	// enable nec7210 dma
	priv->imr2_bits |= HR_DMAI;
	priv->write_byte(priv, priv->imr2_bits, IMR2);

	// attempt to busy wait may improve performance XXX

	// wait for data to transfer
	if(wait_event_interruptible(driver->wait, test_bit(DMA_IN_PROGRESS_BN, &priv->state) == 0 ||
		test_bit(TIMO_NUM, &driver->status)))
	{
		printk("gpib: dma read wait interrupted\n");
		retval = -EINTR;
	}

	// disable nec7210 dma
	priv->imr2_bits &= ~HR_DMAI;
	priv->write_byte(priv, priv->imr2_bits, IMR2);

	// disable 'data in' and 'end' interrupt
	priv->imr1_bits &= ~HR_DIIE & ~HR_ENDIE;
	priv->write_byte(priv, priv->imr1_bits, IMR1);

	// record how many bytes we transferred
	flags = claim_dma_lock();
	clear_dma_ff(priv->dma_channel);
	disable_dma(priv->dma_channel);
	count += length - get_dma_residue(priv->dma_channel);
	release_dma_lock(flags);

	return retval ? retval : count;
}

ssize_t nec7210_read(gpib_driver_t *driver, uint8_t *buffer, size_t length, int *end)
{
	size_t	count = 0;
	ssize_t retval = 0;
	nec7210_private_t *priv = driver->private_data;
	*end = 0;

	if(length == 0) return 0;

	if(test_and_clear_bit(RFD_HOLDOFF_BN, &priv->state))
	{
		priv->write_byte(priv, priv->auxa_bits | HR_HLDA, AUXMR);
		priv->write_byte(priv, AUX_FH, AUXMR);
	}
/*
 *	holdoff on END
 */
	priv->write_byte(priv, priv->auxa_bits | HR_HLDE, AUXMR);

	// transfer data (except for last byte)
	length--;
	if(length)
	{
		if(priv->dma_channel)
		{		// ISA DMA transfer
			retval = dma_read(driver, priv, buffer, length);
		}else
		{	// PIO transfer
			retval = pio_read(driver, priv, buffer, length);
		}
		if(retval < 0)
			return retval;
		else
			count += retval;
	}

	// read last byte if we havn't received an END yet
	if(test_bit(END_NUM, &driver->status) == 0)
	{
		// make sure we holdoff after last byte read
		priv->write_byte(priv, priv->auxa_bits | HR_HLDA, AUXMR);
		retval = pio_read(driver, priv, &buffer[count], 1);
		if(retval < 0)
			return retval;
		else
			count++;
	}

	if(test_and_clear_bit(END_NUM, &driver->status))
		*end = 1;

	set_bit(RFD_HOLDOFF_BN, &priv->state);

	return count;
}








