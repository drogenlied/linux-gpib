/***************************************************************************
                                fmh_gpib.c
                             -------------------
GPIB Driver for fmh_gpib_core, see

https://github.com/fmhess/fmh_gpib_core

More specifically, it is a driver for the hardware arrangement described by
src/examples/fmh_gpib_top.vhd in the fmh_gpib_core repository.

Author: Frank Mori Hess <fmh6jj@gmail.com>
Copyright: (C) 2006, 2010, 2015 Fluke Corporation
	(C) 2017 Frank Mori Hess

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "fmh_gpib.h"

#include "gpibP.h"
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

int fmh_gpib_attach_holdoff_all(gpib_board_t *board, const gpib_board_config_t *config);
int fmh_gpib_attach_holdoff_end(gpib_board_t *board, const gpib_board_config_t *config);
void fmh_gpib_detach(gpib_board_t *board);
static int fmh_gpib_config_dma(gpib_board_t *board, int output);
irqreturn_t fmh_gpib_internal_interrupt(gpib_board_t *board);
static struct platform_driver fmh_gpib_platform_driver;

// wrappers for interface functions
int fmh_gpib_read(gpib_board_t *board, uint8_t *buffer, size_t length, int *end, size_t *bytes_read)
{
	fmh_gpib_private_t *priv = board->private_data;
	return nec7210_read(board, &priv->nec7210_priv, buffer, length, end, bytes_read);
}
int fmh_gpib_write(gpib_board_t *board, uint8_t *buffer, size_t length, int send_eoi, size_t *bytes_written)
{
	fmh_gpib_private_t *priv = board->private_data;
	return nec7210_write(board, &priv->nec7210_priv, buffer, length, send_eoi, bytes_written);
}
int fmh_gpib_command(gpib_board_t *board, uint8_t *buffer, size_t length, size_t *bytes_written)
{
	fmh_gpib_private_t *priv = board->private_data;
	return nec7210_command(board, &priv->nec7210_priv, buffer, length, bytes_written);
}
int fmh_gpib_take_control(gpib_board_t *board, int synchronous)
{
	fmh_gpib_private_t *priv = board->private_data;
	return nec7210_take_control(board, &priv->nec7210_priv, synchronous);
}
int fmh_gpib_go_to_standby(gpib_board_t *board)
{
	fmh_gpib_private_t *priv = board->private_data;
	return nec7210_go_to_standby(board, &priv->nec7210_priv);
}
void fmh_gpib_request_system_control( gpib_board_t *board, int request_control )
{
	fmh_gpib_private_t *priv = board->private_data;
	nec7210_private_t *nec_priv = &priv->nec7210_priv;
	nec7210_request_system_control( board, nec_priv, request_control );
}
void fmh_gpib_interface_clear(gpib_board_t *board, int assert)
{
	fmh_gpib_private_t *priv = board->private_data;
	nec7210_interface_clear(board, &priv->nec7210_priv, assert);
}
void fmh_gpib_remote_enable(gpib_board_t *board, int enable)
{
	fmh_gpib_private_t *priv = board->private_data;
	nec7210_remote_enable(board, &priv->nec7210_priv, enable);
}
int fmh_gpib_enable_eos(gpib_board_t *board, uint8_t eos_byte, int compare_8_bits)
{
	fmh_gpib_private_t *priv = board->private_data;
	return nec7210_enable_eos(board, &priv->nec7210_priv, eos_byte, compare_8_bits);
}
void fmh_gpib_disable_eos(gpib_board_t *board)
{
	fmh_gpib_private_t *priv = board->private_data;
	nec7210_disable_eos(board, &priv->nec7210_priv);
}
unsigned int fmh_gpib_update_status( gpib_board_t *board, unsigned int clear_mask )
{
	fmh_gpib_private_t *priv = board->private_data;
	return nec7210_update_status( board, &priv->nec7210_priv, clear_mask );
}
void fmh_gpib_primary_address(gpib_board_t *board, unsigned int address)
{
	fmh_gpib_private_t *priv = board->private_data;
	nec7210_primary_address(board, &priv->nec7210_priv, address);
}
void fmh_gpib_secondary_address(gpib_board_t *board, unsigned int address, int enable)
{
	fmh_gpib_private_t *priv = board->private_data;
	nec7210_secondary_address(board, &priv->nec7210_priv, address, enable);
}
int fmh_gpib_parallel_poll(gpib_board_t *board, uint8_t *result)
{
	fmh_gpib_private_t *priv = board->private_data;
	return nec7210_parallel_poll(board, &priv->nec7210_priv, result);
}
void fmh_gpib_parallel_poll_configure( gpib_board_t *board, uint8_t configuration )
{
	fmh_gpib_private_t *priv = board->private_data;
	nec7210_parallel_poll_configure(board, &priv->nec7210_priv, configuration );
}
void fmh_gpib_parallel_poll_response( gpib_board_t *board, int ist )
{
	fmh_gpib_private_t *priv = board->private_data;
	nec7210_parallel_poll_response(board, &priv->nec7210_priv, ist );
}
void fmh_gpib_local_parallel_poll_mode( gpib_board_t *board, int local )
{
	fmh_gpib_private_t *priv = board->private_data;
	if(local)
	{
		write_byte(&priv->nec7210_priv, AUX_I_REG | LOCAL_PPOLL_MODE_BIT, AUXMR);
	}else
	{
		/* For fmh_gpib_core, remote parallel poll config mode is unaffected by the
		state of the disable bit of the parallel poll register (unlike the tnt4882).  So,
		we don't need to worry about that. */
		write_byte(&priv->nec7210_priv, AUX_I_REG | 0x0, AUXMR);
	}
}
void fmh_gpib_serial_poll_response(gpib_board_t *board, uint8_t status)
{
	fmh_gpib_private_t *priv = board->private_data;
	nec7210_serial_poll_response(board, &priv->nec7210_priv, status);
}
uint8_t fmh_gpib_serial_poll_status( gpib_board_t *board )
{
	fmh_gpib_private_t *priv = board->private_data;
	return nec7210_serial_poll_status( board, &priv->nec7210_priv );
}
void fmh_gpib_return_to_local( gpib_board_t *board )
{
	fmh_gpib_private_t *priv = board->private_data;
	nec7210_private_t *nec_priv = &priv->nec7210_priv;
	write_byte(nec_priv, AUX_RTL2, AUXMR);
	udelay(1);
	write_byte(nec_priv, AUX_RTL, AUXMR);
}
int fmh_gpib_line_status( const gpib_board_t *board )
{
	int status = ValidALL;
	int bsr_bits;
	fmh_gpib_private_t *e_priv;
	nec7210_private_t *nec_priv;

	e_priv = board->private_data;
	nec_priv = &e_priv->nec7210_priv;

	bsr_bits = read_byte(nec_priv, BUS_STATUS_REG);

	if( ( bsr_bits & BSR_REN_BIT ) == 0 )
		status |= BusREN;
	if( ( bsr_bits & BSR_IFC_BIT ) == 0 )
		status |= BusIFC;
	if( ( bsr_bits & BSR_SRQ_BIT ) == 0 )
		status |= BusSRQ;
	if( ( bsr_bits & BSR_EOI_BIT ) == 0 )
		status |= BusEOI;
	if( ( bsr_bits & BSR_NRFD_BIT ) == 0 )
		status |= BusNRFD;
	if( ( bsr_bits & BSR_NDAC_BIT ) == 0 )
		status |= BusNDAC;
	if( ( bsr_bits & BSR_DAV_BIT ) == 0 )
		status |= BusDAV;
	if( ( bsr_bits & BSR_ATN_BIT ) == 0 )
		status |= BusATN;

	return status;
}

unsigned int fmh_gpib_t1_delay( gpib_board_t *board, unsigned int nano_sec )
{
	fmh_gpib_private_t *e_priv = board->private_data;
	nec7210_private_t *nec_priv = &e_priv->nec7210_priv;
	unsigned int retval;

	retval = nec7210_t1_delay( board, nec_priv, nano_sec );

	if( nano_sec <= 350 )
	{
		write_byte( nec_priv, AUX_HI_SPEED, AUXMR );
		retval = 350;
	}else
		write_byte( nec_priv, AUX_LO_SPEED, AUXMR );

	return retval;
}

static int wait_for_idle(gpib_board_t *board, short wake_on_listener_idle,
	short wake_on_talker_idle)
{
	fmh_gpib_private_t *e_priv = board->private_data;
	nec7210_private_t *nec_priv = &e_priv->nec7210_priv;
	int retval = 0;
// 	printk("%s: enter\n", __FUNCTION__);
	if(wait_event_interruptible(board->wait,
		(wake_on_listener_idle && test_bit(LACS_NUM, &board->status) == 0) ||
		(wake_on_talker_idle && test_bit(TACS_NUM, &board->status) == 0) ||
		test_bit(DEV_CLEAR_BN, &nec_priv->state) ||
		test_bit(TIMO_NUM, &board->status)))
	{
		retval = -ERESTARTSYS;
	}
	if(test_bit(TIMO_NUM, &board->status))
		retval = -ETIMEDOUT;
	if(test_and_clear_bit(DEV_CLEAR_BN, &nec_priv->state))
		retval = -EINTR;
// 	printk("%s: exit, retval=%i\n", __FUNCTION__, retval);
	return retval;
}

/* Wait until the gpib chip is ready to accept a data out byte.
 */
static int wait_for_data_out_ready(gpib_board_t *board)
{
	fmh_gpib_private_t *e_priv = board->private_data;
	nec7210_private_t *nec_priv = &e_priv->nec7210_priv;
	int retval = 0;
// 	printk("%s: enter\n", __FUNCTION__);

	if(wait_event_interruptible(board->wait,
		(gpib_cs_read_byte(nec_priv, EXT_STATUS_1_REG) & DATA_OUT_STATUS_BIT) ||
		test_bit(DEV_CLEAR_BN, &nec_priv->state) ||
		test_bit(TIMO_NUM, &board->status)))
	{
		retval = -ERESTARTSYS;
	}
	if(test_bit(TIMO_NUM, &board->status))
		retval = -ETIMEDOUT;
	if(test_and_clear_bit(DEV_CLEAR_BN, &nec_priv->state))
		retval = -EINTR;
// 	printk("%s: exit, retval=%i\n", __FUNCTION__, retval);
	return retval;
}

static void fmh_gpib_dma_callback(void *arg)
{
	gpib_board_t *board = arg;
	fmh_gpib_private_t *e_priv = board->private_data;
	nec7210_private_t *nec_priv = &e_priv->nec7210_priv;
	unsigned long flags;
	spin_lock_irqsave(&board->spinlock, flags);
//	printk("%s: enter\n", __FUNCTION__);

	nec7210_set_reg_bits(nec_priv, IMR1, HR_DOIE | HR_DIIE, HR_DOIE | HR_DIIE);
	wake_up_interruptible(&board->wait);

	fmh_gpib_internal_interrupt(board);
	clear_bit(DMA_WRITE_IN_PROGRESS_BN, &nec_priv->state);
	clear_bit(DMA_READ_IN_PROGRESS_BN, &nec_priv->state);
//	printk("%s: exit\n", __FUNCTION__);
	spin_unlock_irqrestore(&board->spinlock, flags);
}

static int fmh_gpib_dma_write(gpib_board_t *board, 
	uint8_t *buffer, size_t length, size_t *bytes_written)
{
	fmh_gpib_private_t *e_priv = board->private_data;
	nec7210_private_t *nec_priv = &e_priv->nec7210_priv;
	unsigned long flags;
	int retval = 0;
	dma_addr_t address;
	struct dma_async_tx_descriptor *tx_desc;
	
	*bytes_written = 0;
// 	printk("%s: enter\n", __FUNCTION__);
	if(length > e_priv->dma_buffer_size)
		BUG();
	dmaengine_terminate_all(e_priv->dma_channel);
	retval = wait_for_idle(board, 1, 0);
	if(retval < 0) return retval;
	memcpy(e_priv->dma_buffer, buffer, length);
	address = dma_map_single(NULL, e_priv->dma_buffer,
		 length, DMA_TO_DEVICE);
	if(dma_mapping_error(NULL,  address))
	{
		printk("dma mapping error in dma write!\n");
	}
	/* program dma controller */
	retval = fmh_gpib_config_dma(board, 1);
	if(retval) goto cleanup;

	tx_desc = dmaengine_prep_slave_single(e_priv->dma_channel, address, length, DMA_MEM_TO_DEV, 
			DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if(tx_desc == NULL)
	{
		printk("fmh_gpib_gpib: failed to allocate dma transmit descriptor\n");
		retval = -ENOMEM;
		goto cleanup;
	}
	tx_desc->callback = fmh_gpib_dma_callback;
	tx_desc->callback_param = board;
	
	spin_lock_irqsave(&board->spinlock, flags);
	fifos_write(e_priv, length & fifo_xfer_counter_mask, FIFO_XFER_COUNTER_REG);
	fifos_write(e_priv, TX_FIFO_DMA_REQUEST_ENABLE | TX_FIFO_CLEAR, FIFO_CONTROL_STATUS_REG); 
	nec7210_set_reg_bits(nec_priv, IMR1, HR_DOIE, 0);
	nec7210_set_reg_bits(nec_priv, IMR2, HR_DMAO, HR_DMAO);

	dmaengine_submit(tx_desc);
	dma_async_issue_pending(e_priv->dma_channel);
	clear_bit(WRITE_READY_BN, &nec_priv->state);
	set_bit(DMA_WRITE_IN_PROGRESS_BN, &nec_priv->state);
// 	printk("%s: in spin lock\n", __FUNCTION__);
	spin_unlock_irqrestore(&board->spinlock, flags);

//	printk("%s: waiting for write.\n", __FUNCTION__);
	// suspend until message is sent
	if(wait_event_interruptible(board->wait, 
	   ((fifos_read(e_priv, FIFO_XFER_COUNTER_REG) & fifo_xfer_counter_mask) == 0 
			   /*&& test_bit(WRITE_READY_BN, &nec_priv->state) */) ||
		test_bit(BUS_ERROR_BN, &nec_priv->state) || 
		test_bit(DEV_CLEAR_BN, &nec_priv->state) ||
		test_bit(TIMO_NUM, &board->status)))
	{
		GPIB_DPRINTK( "gpib write interrupted!\n" );
		retval = -ERESTARTSYS;
	}
	if(test_bit(TIMO_NUM, &board->status))
		retval = -ETIMEDOUT;
	if(test_and_clear_bit(DEV_CLEAR_BN, &nec_priv->state))
		retval = -EINTR;
	if(test_and_clear_bit(BUS_ERROR_BN, &nec_priv->state))
		retval = -EIO;
	// disable board's dma
	nec7210_set_reg_bits(nec_priv, IMR2, HR_DMAO, 0);
	fifos_write(e_priv, 0, FIFO_CONTROL_STATUS_REG); 

	dmaengine_terminate_all(e_priv->dma_channel);
	// make sure fmh_gpib_dma_callback got called
	if(test_bit(DMA_WRITE_IN_PROGRESS_BN, &nec_priv->state))
	{
		fmh_gpib_dma_callback(board);
	}

	*bytes_written = length - (fifos_read(e_priv, FIFO_XFER_COUNTER_REG) & fifo_xfer_counter_mask);
	if(*bytes_written > length) BUG();
	/*	printk("length=%i, *bytes_written=%i, residue=%i, retval=%i\n",
		length, *bytes_written, get_dma_residue(e_priv->dma_channel), retval);*/
cleanup:
	dma_unmap_single(NULL, address, length, DMA_TO_DEVICE);
//	printk("%s: exit, retval=%d\n", __FUNCTION__, retval);
	return retval;
}

static int fmh_gpib_accel_write(gpib_board_t *board, 
	uint8_t *buffer, size_t length, int send_eoi, size_t *bytes_written)
{
	fmh_gpib_private_t *e_priv = board->private_data;
	nec7210_private_t *nec_priv = &e_priv->nec7210_priv;
	size_t remainder = length;
	size_t transfer_size;
	ssize_t retval = 0;
	size_t dma_remainder = remainder;
	
	if(e_priv->dma_channel == NULL) 
	{
		printk("fmh_gpib_gpib: No dma channel available, cannot do accel write.");
		return -ENXIO;
	}
	
	*bytes_written = 0;
	if(length < 1) return 0;
	clear_bit(DEV_CLEAR_BN, &nec_priv->state); // XXX FIXME
	if(send_eoi) --dma_remainder;
// 	printk("%s: entering while loop\n", __FUNCTION__);
	
	while(dma_remainder > 0)
	{
		size_t num_bytes;
		transfer_size = (e_priv->dma_buffer_size < dma_remainder) ? 
			e_priv->dma_buffer_size : dma_remainder;
		retval = fmh_gpib_dma_write(board, buffer, transfer_size, &num_bytes);
		*bytes_written += num_bytes;
		if(retval < 0) break;
		dma_remainder -= num_bytes;
		remainder -= num_bytes;
		buffer += num_bytes;
		if(need_resched()) schedule();
	}
	if(retval < 0) return retval;
	//handle sending of last byte with eoi
	if(send_eoi)
	{
		size_t num_bytes;
		// 		printk("%s: handling last byte\n", __FUNCTION__);
		if(remainder != 1) BUG();
		
		/* wait until we are sure we will be able to write the data byte
		 * into the chip before we send AUX_SEOI.  This prevents a timeout
		 * scenerio where we send AUX_SEOI but then timeout without getting
		 * any bytes into the gpib chip.  This will result in the first byte
		 * of the next write having a spurious EOI set on the first byte. */
		retval = wait_for_data_out_ready(board);
		if(retval < 0) return retval;
		
		write_byte(nec_priv, AUX_SEOI, AUXMR);
		retval = fmh_gpib_dma_write(board, buffer, remainder, &num_bytes);
		*bytes_written += num_bytes;
		if(retval < 0) return retval;
		remainder -= num_bytes;
	}
// 	printk("%s: bytes send=%i\n", __FUNCTION__, (int)(length - remainder));
	return 0;
}

static unsigned fmh_gpib_get_dma_residue(struct dma_chan *chan, dma_cookie_t cookie)
{
	struct dma_tx_state state;
	int result;
	
	result = dmaengine_pause(chan);
	if(result < 0)
	{
		printk("fmh_gpib_gpib: dma pause failed?\n");
		BUG();
	}
	dmaengine_tx_status(chan, cookie, &state);
	// hardware doesn't support resume, so dont call this
	// method unless the dma transfer is done.
	return state.residue;
}

static int fmh_gpib_dma_read(gpib_board_t *board, uint8_t *buffer,
	size_t length, int *end, size_t *bytes_read)
{
	fmh_gpib_private_t *e_priv = board->private_data;
	nec7210_private_t *nec_priv = &e_priv->nec7210_priv;
	int retval = 0;
	unsigned long flags;
	unsigned residue;
	int wait_retval;
	dma_addr_t bus_address;
	struct dma_async_tx_descriptor *tx_desc;
	dma_cookie_t dma_cookie;
	int i;
	const int timeout = 100;
	
	// 	printk("%s: enter, bus_address=0x%x, length=%i\n", __FUNCTION__, (unsigned)bus_address,
// 		   (int)length);

	*bytes_read = 0;
	*end = 0;
	if(length == 0)
		return 0;

	retval = wait_for_idle(board, 0, 1);
	if(retval < 0) return retval;
	bus_address = dma_map_single(NULL, e_priv->dma_buffer,
		length, DMA_FROM_DEVICE);
	if(dma_mapping_error(NULL, bus_address))
	{
		printk("dma mapping error in dma read!");
	}

	/* program dma controller */
	retval = fmh_gpib_config_dma(board, 0);
	if(retval) 
	{
		dma_unmap_single(NULL, bus_address, length, DMA_FROM_DEVICE);
		return retval;
	}
	tx_desc = dmaengine_prep_slave_single(e_priv->dma_channel, bus_address, length, DMA_DEV_TO_MEM, 
		DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if(tx_desc == NULL)
	{
		printk("fmh_gpib_gpib: failed to allocate dma transmit descriptor\n");
		dma_unmap_single(NULL, bus_address, length, DMA_FROM_DEVICE);
		return -EIO;
	}
	tx_desc->callback = fmh_gpib_dma_callback;
	tx_desc->callback_param = board;

	spin_lock_irqsave(&board->spinlock, flags);
	// enable nec7210 dma
	fifos_write(e_priv, length & fifo_xfer_counter_mask, FIFO_XFER_COUNTER_REG);
	fifos_write(e_priv, RX_FIFO_DMA_REQUEST_ENABLE | RX_FIFO_CLEAR, FIFO_CONTROL_STATUS_REG); 
	nec7210_set_reg_bits(nec_priv, IMR1, HR_DIIE, 0);
	nec7210_set_reg_bits(nec_priv, IMR2, HR_DMAI, HR_DMAI);

	dma_cookie = dmaengine_submit(tx_desc);
	dma_async_issue_pending(e_priv->dma_channel);

	set_bit(DMA_READ_IN_PROGRESS_BN, &nec_priv->state);
	clear_bit(READ_READY_BN, &nec_priv->state);
	
	spin_unlock_irqrestore(&board->spinlock, flags);
// 	printk("waiting for data transfer.\n");
	// wait for data to transfer
	if((wait_retval = wait_event_interruptible(board->wait,
		test_bit(DMA_READ_IN_PROGRESS_BN, &nec_priv->state) == 0 ||
		test_bit(RECEIVED_END_BN, &nec_priv->state) ||
		test_bit(DEV_CLEAR_BN, &nec_priv->state) ||
		test_bit(TIMO_NUM, &board->status))))
	{
		printk("fluke: dma read wait interrupted\n");
		retval = -ERESTARTSYS;
	}
	if(test_bit(TIMO_NUM, &board->status))
		retval = -ETIMEDOUT;
	if(test_bit(DEV_CLEAR_BN, &nec_priv->state))
		retval = -EINTR;
	// stop the dma transfer
	nec7210_set_reg_bits(nec_priv, IMR2, HR_DMAI, 0);
	for(i = 0; 
		((fifos_read(e_priv, FIFO_CONTROL_STATUS_REG) & RX_FIFO_EMPTY) == 0) && i < timeout; 
		++i)
	{
		udelay(10);
	}
	if(i == timeout)
	{
		dev_err(board->dev, "timed out waiting for rx fifo to empty!\n");
		printk("timeout: 0x%x\n", (unsigned)fifos_read(e_priv, FIFO_CONTROL_STATUS_REG)); 
	}
	/* delay a little just to make sure any bytes in dma controller's fifo get
	 written to memory before we disable it */
	udelay(10);
	fifos_write(e_priv, 0, FIFO_CONTROL_STATUS_REG); 
	residue = fmh_gpib_get_dma_residue(e_priv->dma_channel, dma_cookie);
	BUG_ON(residue > length);
	*bytes_read += length - residue;
	dmaengine_terminate_all(e_priv->dma_channel);
	// make sure fmh_gpib_dma_callback got called
	if(test_bit(DMA_READ_IN_PROGRESS_BN, &nec_priv->state))
	{
		fmh_gpib_dma_callback(board);
	}

	dma_unmap_single(NULL, bus_address, length, DMA_FROM_DEVICE);
	memcpy(buffer, e_priv->dma_buffer, *bytes_read);

	/* If we got an end interrupt, figure out if it was
	 * associated with the last byte we dma'd or with a
	 * byte still sitting on the cb7210.
	 */
	spin_lock_irqsave(&board->spinlock, flags);
	if(read_byte( nec_priv, EXT_STATUS_1_REG ) & DATA_IN_STATUS_BIT)
	{
		set_bit(READ_READY_BN, &nec_priv->state);
	}else
	{
		clear_bit(READ_READY_BN, &nec_priv->state);
		// There is no byte sitting on the cb7210.  If we
		// saw an end interrupt, we need to deal with it now
		if(test_and_clear_bit(RECEIVED_END_BN, &nec_priv->state)) 
		{
			*end = 1;
		}
	}
	spin_unlock_irqrestore(&board->spinlock, flags);
// 	printk("\tbytes_read=%i, residue=%i, end=%i, retval=%i, wait_retval=%i\n", 
// 		   *bytes_read, residue, *end, retval, wait_retval);

	return retval;
}

static int fmh_gpib_accel_read(gpib_board_t *board, uint8_t *buffer, size_t length,
	int *end, size_t *bytes_read)
{
	fmh_gpib_private_t *e_priv = board->private_data;
	nec7210_private_t *nec_priv = &e_priv->nec7210_priv;
	size_t remain = length;
	size_t transfer_size;
	int retval = 0;
	size_t dma_nbytes;
/*	printk("%s: enter, buffer=0x%p, length=%i\n", __FUNCTION__,
		   buffer, (int)length);
	printk("\t dma_buffer=0x%p\n", e_priv->dma_buffer);*/
	clear_bit(DEV_CLEAR_BN, &nec_priv->state); // XXX FIXME
	*end = 0;
	*bytes_read = 0;
	nec7210_release_rfd_holdoff(board, nec_priv);
// 	printk("%s: entering while loop\n", __FUNCTION__);
	while(remain > 0)
	{
		transfer_size = (e_priv->dma_buffer_size < remain) ? e_priv->dma_buffer_size : remain;
		retval = fmh_gpib_dma_read(board, buffer, transfer_size, end, &dma_nbytes);
		remain -= dma_nbytes;
		buffer += dma_nbytes;
		*bytes_read += dma_nbytes;
		if(*end) 
		{
			break;
		}
		if(retval < 0) 
		{
// 			printk("%s: early exit, retval=%i\n", __FUNCTION__, (int)retval);
			return retval;
		}
		if(need_resched()) schedule();
	}
// 	printk("%s: exit, retval=%i\n", __FUNCTION__, (int)retval);
	return retval;
}

gpib_interface_t fmh_gpib_unaccel_interface =
{
	name: "fmh_gpib_unaccel",
	attach: fmh_gpib_attach_holdoff_all,
	detach: fmh_gpib_detach,
	read: fmh_gpib_read,
	write: fmh_gpib_write,
	command: fmh_gpib_command,
	take_control: fmh_gpib_take_control,
	go_to_standby: fmh_gpib_go_to_standby,
	request_system_control: fmh_gpib_request_system_control,
	interface_clear: fmh_gpib_interface_clear,
	remote_enable: fmh_gpib_remote_enable,
	enable_eos: fmh_gpib_enable_eos,
	disable_eos: fmh_gpib_disable_eos,
	parallel_poll: fmh_gpib_parallel_poll,
	parallel_poll_configure: fmh_gpib_parallel_poll_configure,
	parallel_poll_response: fmh_gpib_parallel_poll_response,
	local_parallel_poll_mode: fmh_gpib_local_parallel_poll_mode,
	line_status: fmh_gpib_line_status,
	update_status: fmh_gpib_update_status,
	primary_address: fmh_gpib_primary_address,
	secondary_address: fmh_gpib_secondary_address,
	serial_poll_response: fmh_gpib_serial_poll_response,
	serial_poll_status: fmh_gpib_serial_poll_status,
	t1_delay: fmh_gpib_t1_delay,
	return_to_local: fmh_gpib_return_to_local,
};

gpib_interface_t fmh_gpib_interface =
{
	name: "fmh_gpib",
	attach: fmh_gpib_attach_holdoff_end,
	detach: fmh_gpib_detach,
	read: fmh_gpib_accel_read,
	write: fmh_gpib_accel_write,
	command: fmh_gpib_command,
	take_control: fmh_gpib_take_control,
	go_to_standby: fmh_gpib_go_to_standby,
	request_system_control: fmh_gpib_request_system_control,
	interface_clear: fmh_gpib_interface_clear,
	remote_enable: fmh_gpib_remote_enable,
	enable_eos: fmh_gpib_enable_eos,
	disable_eos: fmh_gpib_disable_eos,
	parallel_poll: fmh_gpib_parallel_poll,
	parallel_poll_configure: fmh_gpib_parallel_poll_configure,
	parallel_poll_response: fmh_gpib_parallel_poll_response,
	local_parallel_poll_mode: fmh_gpib_local_parallel_poll_mode,
	line_status: fmh_gpib_line_status,
	update_status: fmh_gpib_update_status,
	primary_address: fmh_gpib_primary_address,
	secondary_address: fmh_gpib_secondary_address,
	serial_poll_response: fmh_gpib_serial_poll_response,
	serial_poll_status: fmh_gpib_serial_poll_status,
	t1_delay: fmh_gpib_t1_delay,
	return_to_local: fmh_gpib_return_to_local,
};

irqreturn_t fmh_gpib_internal_interrupt(gpib_board_t *board)
{
	int status0, status1, status2, ext_status_1;
	fmh_gpib_private_t *priv = board->private_data;
	nec7210_private_t *nec_priv = &priv->nec7210_priv;
	int retval = IRQ_NONE;

	status0 = read_byte( nec_priv, ISR0_IMR0_REG );
	status1 = read_byte( nec_priv, ISR1 );
	status2 = read_byte( nec_priv, ISR2 );
	ext_status_1 = read_byte(nec_priv, EXT_STATUS_1_REG);
	
	if(status0 & IFC_INTERRUPT_BIT)
	{
		push_gpib_event( board, EventIFC );
		retval = IRQ_HANDLED;
	}
	
	if( nec7210_interrupt_have_status(board, nec_priv, status1, status2) == IRQ_HANDLED)
		retval = IRQ_HANDLED;

	if(ext_status_1 & DATA_IN_STATUS_BIT)
		set_bit(READ_READY_BN, &nec_priv->state);
	else
		clear_bit(READ_READY_BN, &nec_priv->state);
	
	if(ext_status_1 & DATA_OUT_STATUS_BIT)
		set_bit(WRITE_READY_BN, &nec_priv->state);
	else
		clear_bit(WRITE_READY_BN, &nec_priv->state);

	if(ext_status_1 & COMMAND_OUT_STATUS_BIT)
		set_bit(COMMAND_READY_BN, &nec_priv->state);
	else
		clear_bit(COMMAND_READY_BN, &nec_priv->state);

	if( retval == IRQ_HANDLED )
	{
		wake_up_interruptible( &board->wait );
	}
    
	return retval;
}

irqreturn_t fmh_gpib_interrupt(int irq, void *arg)
{
	gpib_board_t *board = arg;
	unsigned long flags;
	irqreturn_t retval;
	
	spin_lock_irqsave(&board->spinlock, flags);
	retval = fmh_gpib_internal_interrupt(board);
	spin_unlock_irqrestore(&board->spinlock, flags);
	return retval;
}

int fmh_gpib_allocate_private(gpib_board_t *board)
{
	fmh_gpib_private_t *priv;

	board->private_data = kmalloc(sizeof(fmh_gpib_private_t), GFP_KERNEL);
	if(board->private_data == NULL)
		return -ENOMEM;
	priv = board->private_data;
	memset( priv, 0, sizeof(fmh_gpib_private_t));
	init_nec7210_private(&priv->nec7210_priv);
	priv->dma_buffer_size = 0x800;
	priv->dma_buffer = kmalloc(priv->dma_buffer_size, GFP_KERNEL);
	if(priv->dma_buffer == NULL)
		return -ENOMEM;
	return 0;
}

void fmh_gpib_generic_detach(gpib_board_t *board)
{
	if(board->private_data)
	{
		fmh_gpib_private_t *e_priv = board->private_data;
		if(e_priv->dma_buffer)
			kfree(e_priv->dma_buffer);
		kfree(board->private_data);
		board->private_data = NULL;
	}
	if(board->dev != NULL)
		dev_set_drvdata(board->dev, NULL);
}

// generic part of attach functions
int fmh_gpib_generic_attach(gpib_board_t *board)
{
	fmh_gpib_private_t *e_priv;
	nec7210_private_t *nec_priv;
	int retval;

	board->status = 0;

	retval = fmh_gpib_allocate_private(board);
	if(retval < 0)
		return retval;
	e_priv = board->private_data;
	nec_priv = &e_priv->nec7210_priv;
	nec_priv->read_byte = gpib_cs_read_byte;
	nec_priv->write_byte = gpib_cs_write_byte;
	nec_priv->offset = gpib_cs_reg_offset;
	nec_priv->type = CB7210;
	return 0;
}

static int fmh_gpib_config_dma(gpib_board_t *board, int output)
{
	fmh_gpib_private_t *e_priv = board->private_data;
	struct dma_slave_config config;
	config.device_fc = true;
	config.slave_id = 0;
	config.src_maxburst = 1;
	config.dst_maxburst = 1;
	
	if(output)
	{
		config.direction = DMA_MEM_TO_DEV;
		config.src_addr = 0;
		config.dst_addr = e_priv->dma_port_res->start + FIFO_DATA_REG * fifo_reg_offset;
		config.src_addr_width = 1;
		config.dst_addr_width = 1;
	}else
	{
		config.direction = DMA_DEV_TO_MEM;
		config.src_addr = e_priv->dma_port_res->start + FIFO_DATA_REG * fifo_reg_offset;
		config.dst_addr = 0;
		config.src_addr_width = 1;
		config.dst_addr_width = 1;
	}
	return dmaengine_slave_config(e_priv->dma_channel, &config);
}

int fmh_gpib_init(fmh_gpib_private_t *e_priv, gpib_board_t *board, int handshake_mode)
{
	nec7210_private_t *nec_priv = &e_priv->nec7210_priv;

	fifos_write(e_priv, 0, FIFO_CONTROL_STATUS_REG); 

	nec7210_board_reset(nec_priv, board);
	write_byte(nec_priv, AUX_LO_SPEED, AUXMR);
	nec7210_set_handshake_mode(board, nec_priv, handshake_mode);

	nec7210_board_online( nec_priv, board );

	write_byte(nec_priv, IFC_INTERRUPT_ENABLE_BIT | ATN_INTERRUPT_ENABLE_BIT, ISR0_IMR0_REG);
	return 0;
}

/* This function is passed to dma_request_channel() in order to
 * select the pl330 dma channel which has been hardwired to
 * the gpib controller. */
static bool gpib_dma_channel_filter(struct dma_chan *chan, void *filter_param)
{
	s32 *dma_channel_id = (s32*)filter_param;
	// select the channel which is wired to the gpib chip
	return chan->chan_id == *dma_channel_id;
}

/* Match callback for driver_find_device */
static int fmh_gpib_device_match(struct device *dev, void *data)
{
	const gpib_board_config_t *config = data;
	struct device_node *of_node;
	
	if(dev_get_drvdata(dev) != NULL) return 0;
	else
	{
		if(config->device_tree_path != NULL)
		{
			of_node = of_find_node_by_path(config->device_tree_path);
			if(of_node == NULL || of_node != dev_of_node(dev))
			{
				return 0;
			}
		}
		dev_notice(dev, "matched: %s\n", of_node_full_name(dev_of_node((dev))));
		return 1;
	}
}

static int fmh_gpib_attach_impl(gpib_board_t *board, const gpib_board_config_t *config, unsigned handshake_mode, int acquire_dma)
{
	fmh_gpib_private_t *e_priv;
	nec7210_private_t *nec_priv;
	int isr_flags = 0;
	int retval;
	int irq;
	struct resource *res;
	dma_cap_mask_t dma_cap;
	struct platform_device *pdev;
	s32 dma_channel_id;
	
	board->dev = driver_find_device(&fmh_gpib_platform_driver.driver,
		NULL, (void*)config, &fmh_gpib_device_match);
	if(board->dev == NULL)
	{
		printk("No matching fmh_gpib_core device was found, attach failed.");
		return -ENODEV;
	}
	dev_set_drvdata(board->dev, board); // currently only used to mark the device as already attached
	pdev = to_platform_device(board->dev);
	
	retval = fmh_gpib_generic_attach(board);
	if(retval) return retval;

	e_priv = board->private_data;
	nec_priv = &e_priv->nec7210_priv;
	
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "gpib_control_status");
	if (!res) {
		dev_err(board->dev, "Unable to locate mmio resource for cb7210 gpib\n");
		return -ENODEV;
	}

	if(request_mem_region(res->start,
			resource_size(res),
			pdev->name) == NULL) {
		dev_err(board->dev, "cannot claim registers\n");
		return -ENXIO;
	}
	e_priv->gpib_iomem_res = res;

	nec_priv->iobase = ioremap_nocache(e_priv->gpib_iomem_res->start, 
			resource_size(e_priv->gpib_iomem_res));
	dev_info(board->dev, "iobase 0x%lx remapped to %p, length=%ld\n", (unsigned long)e_priv->gpib_iomem_res->start,
		nec_priv->iobase, (unsigned long)resource_size(e_priv->gpib_iomem_res));
	if (!nec_priv->iobase) {
		dev_err(board->dev, "Could not map I/O memory\n");
		return -ENOMEM;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dma_fifos");
	if (!res) {
		dev_err(board->dev, "Unable to locate mmio resource for gpib dma port\n");
		return -ENODEV;
	}
	if(request_mem_region(res->start,
			resource_size(res),
			pdev->name) == NULL) {
		dev_err(board->dev, "cannot claim registers\n");
		return -ENXIO;
	}
	e_priv->dma_port_res = res;
	e_priv->fifo_base = ioremap_nocache(e_priv->dma_port_res->start, 
			resource_size(e_priv->dma_port_res));
	dev_info(board->dev, "dma fifos 0x%lx remapped to %p, length=%ld\n",
		(unsigned long)e_priv->dma_port_res->start, e_priv->fifo_base, 
		(unsigned long)resource_size(e_priv->dma_port_res));
	
	
	irq = platform_get_irq(pdev, 0);
	printk("gpib: irq %d\n", irq);
	if(irq < 0)
	{
		dev_err(board->dev, "fmh_gpib_gpib: request for IRQ failed\n");
		return -EBUSY;
	}
	retval = request_irq(irq, fmh_gpib_interrupt, isr_flags, pdev->name, board);
	if(retval){
		dev_err(board->dev,
			"cannot register interrupt handler err=%d\n",
				retval);
		return retval;
	}
	e_priv->irq = irq;

	if(acquire_dma)
	{
		retval = of_property_read_s32(dev_of_node(board->dev),
			"dma-channel",
			&dma_channel_id);
		if(retval < 0)
		{
			dev_err(board->dev,
				"failed to read \"dma-channel\" property from device tree entry, err=%d\n",
				retval);
			return retval;
		}else
		{
			dma_cap_zero(dma_cap);
			dma_cap_set(DMA_SLAVE, dma_cap);
			e_priv->dma_channel = dma_request_channel(dma_cap, gpib_dma_channel_filter, &dma_channel_id);
			if(e_priv->dma_channel == NULL) 
			{
				dev_err(board->dev, "failed to allocate a dma channel.\n");
				return -EIO;
			}
		}
	}
	return fmh_gpib_init(e_priv, board, handshake_mode);
}

int fmh_gpib_attach_holdoff_all(gpib_board_t *board, const gpib_board_config_t *config)
{
	return fmh_gpib_attach_impl(board, config, HR_HLDA, 0);
}

int fmh_gpib_attach_holdoff_end(gpib_board_t *board, const gpib_board_config_t *config)
{
	return fmh_gpib_attach_impl(board, config, HR_HLDE, 1);
}

void fmh_gpib_detach(gpib_board_t *board)
{
	fmh_gpib_private_t *e_priv = board->private_data;
	nec7210_private_t *nec_priv;

	if(e_priv)
	{
		if(e_priv->dma_channel != NULL)
			dma_release_channel(e_priv->dma_channel);
		nec_priv = &e_priv->nec7210_priv;

		if(e_priv->fifo_base != NULL)
		{
			fifos_write(e_priv, 0, FIFO_CONTROL_STATUS_REG); 
		}
		if(nec_priv->iobase)
		{
			write_byte(nec_priv, 0, ISR0_IMR0_REG);
			nec7210_board_reset(nec_priv, board);
		}
		if(e_priv->irq)
		{
			free_irq(e_priv->irq, board);
		}
		if(e_priv->dma_port_res)
		{
			release_mem_region(e_priv->dma_port_res->start, 
				resource_size(e_priv->dma_port_res));
		}
		if(e_priv->gpib_iomem_res)
		{
			release_mem_region(e_priv->gpib_iomem_res->start, 
				resource_size(e_priv->gpib_iomem_res));
		}
	}
	fmh_gpib_generic_detach(board);
}

static int fmh_gpib_probe(struct platform_device *pdev) {
	return 0;
}

static const struct of_device_id fmh_gpib_of_match[] = {
	{ .compatible = "fmhess,fmh_gpib_core"},
	{ {0} }
};
MODULE_DEVICE_TABLE(of, fmh_gpib_of_match);

static struct platform_driver fmh_gpib_platform_driver = {
	.driver = {
		.name = "fmh_gpib",
		.owner = THIS_MODULE,
		.of_match_table = fmh_gpib_of_match,
	},
	.probe = &fmh_gpib_probe
};

static int __init fmh_gpib_init_module( void )
{
	int result;
	
	result = platform_driver_register(&fmh_gpib_platform_driver);
	if (result) {
		printk("fmh_gpib_gpib: platform_driver_register failed!\n");
		return result;
	}
	
	gpib_register_driver(&fmh_gpib_unaccel_interface, THIS_MODULE);
	gpib_register_driver(&fmh_gpib_interface, THIS_MODULE);

	printk("fmh_gpib_gpib\n");
	return 0;
}

static void __exit fmh_gpib_exit_module( void )
{
	gpib_unregister_driver(&fmh_gpib_unaccel_interface);
	gpib_unregister_driver(&fmh_gpib_interface);
	platform_driver_unregister(&fmh_gpib_platform_driver);
}

module_init( fmh_gpib_init_module );
module_exit( fmh_gpib_exit_module );
