/***************************************************************************
                          agilent_82357a/agilent_82357a.c  
                             -------------------
 driver for Agilent 82357A usb to gpib adapters

    begin                : 2004-10-31
    copyright            : (C) 2004 by Frank Mori Hess
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

#include <linux/module.h>
#include "agilent_82357a.h"
#include "gpibP.h"
#include "tms9914.h"

MODULE_LICENSE("GPL");

#define MAX_NUM_82357A_INTERFACES 128
static struct usb_interface *agilent_82357a_driver_interfaces[MAX_NUM_82357A_INTERFACES];
static DECLARE_MUTEX(agilent_82357a_hotplug_lock);

//calculates a reasonable timeout in jiffies that can be passed to usb functions
static inline int agilent_82357a_timeout_jiffies(unsigned int usec)
{
	return HZ + (usec * HZ ) / 900000;
};

int agilent_82357a_send_bulk_msg(agilent_82357a_private_t *a_priv, void *data, int data_length, int *actual_data_length, 
	int timeout_jiffies)
{
	struct usb_device *usb_dev;
	int retval;
	unsigned int out_pipe;

	retval = down_interruptible(&a_priv->bulk_transfer_lock);
	if(retval) return retval;
	if(a_priv->bus_interface == NULL)
	{
		up(&a_priv->bulk_transfer_lock);
		return -ENODEV;
	}
	usb_dev = interface_to_usbdev(a_priv->bus_interface);
	out_pipe = usb_sndbulkpipe(usb_dev, AGILENT_82357A_BULK_OUT_ENDPOINT);
	retval = usb_bulk_msg(usb_dev, out_pipe, data, data_length, actual_data_length, timeout_jiffies);
	up(&a_priv->bulk_transfer_lock);
	return retval;
}

int agilent_82357a_receive_bulk_msg(agilent_82357a_private_t *a_priv, void *data, int data_length, int *actual_data_length, 
	int timeout_jiffies)
{
	struct usb_device *usb_dev;
	int retval;
	unsigned int in_pipe;

	retval = down_interruptible(&a_priv->bulk_transfer_lock);
	if(retval) return retval;
	if(a_priv->bus_interface == NULL)
	{
		up(&a_priv->bulk_transfer_lock);
		return -ENODEV;
	}
	
	usb_dev = interface_to_usbdev(a_priv->bus_interface);
	in_pipe = usb_rcvbulkpipe(usb_dev, AGILENT_82357A_BULK_IN_ENDPOINT);
	retval = usb_bulk_msg(usb_dev, in_pipe, data, data_length, actual_data_length, timeout_jiffies);
	up(&a_priv->bulk_transfer_lock);
	return retval;
}

int agilent_82357a_receive_control_msg(agilent_82357a_private_t *a_priv, __u8 request, __u8 requesttype, __u16 value, 
	__u16 index, void *data, __u16 size, int timeout_jiffies)
{
	struct usb_device *usb_dev;
	int retval;
	unsigned int in_pipe;
	
	retval = down_interruptible(&a_priv->control_transfer_lock);
	if(retval) return retval;
	if(a_priv->bus_interface == NULL)
	{
		up(&a_priv->control_transfer_lock);
		return -ENODEV;
	}
	usb_dev = interface_to_usbdev(a_priv->bus_interface);
	in_pipe = usb_rcvctrlpipe(usb_dev, AGILENT_82357A_CONTROL_ENDPOINT);
	retval = usb_control_msg(usb_dev, in_pipe, request, requesttype, value, index, data, size, timeout_jiffies);
	up(&a_priv->control_transfer_lock);
	return retval;
}

static void agilent_82357a_dump_raw_block(const uint8_t *raw_data, int length)
{
	int i;
	
	printk("%s:", __FUNCTION__);
	for(i = 0; i < length; i++)
	{
		if(i % 8 == 0)
			printk("\n"); 
		printk(" %2x", raw_data[i]);
	}
	printk("\n");
}

int agilent_82357a_write_registers(agilent_82357a_private_t *a_priv, const struct agilent_82357a_register_pairlet *writes, 
	int num_writes)
{
	int retval;
	uint8_t *out_data, *in_data;
	int out_data_length, in_data_length;
	int bytes_written, bytes_read;
	int i = 0;
	int j;
	static const int bytes_per_write = 2;
	static const int header_length = 2;
	static const int max_writes = 31;
	
	if(num_writes > max_writes)
	{
		printk("%s: %s: bug! num_writes=%i too large\n", __FILE__, __FUNCTION__, num_writes);
	}
	out_data_length = num_writes * bytes_per_write + header_length;
	out_data = kmalloc(out_data_length, GFP_KERNEL);
	if(out_data == NULL)
	{	
		printk("%s: %s: kmalloc failed\n", __FILE__, __FUNCTION__);
		return -ENOMEM;
	}
	out_data[i++] = DATA_PIPE_CMD_WR_REGS;
	out_data[i++] = num_writes;
	for(j = 0; j < num_writes; j++)
	{	
		out_data[i++] = writes[j].address;
		out_data[i++] = writes[j].value;
	}	
	if(i > out_data_length)
	{
		printk("%s: bug! buffer overrun\n", __FUNCTION__);
	}
	retval = agilent_82357a_send_bulk_msg(a_priv, out_data, i, &bytes_written, HZ);
	kfree(out_data);
	if(retval)
	{
		printk("%s: %s: agilent_82357a_send_bulk_msg returned %i, bytes_written=%i, i=%i\n", __FILE__, __FUNCTION__, 
			retval, bytes_written, i);		
		return retval;
	}
	in_data_length = 0x20;
	in_data = kmalloc(in_data_length, GFP_KERNEL);
	if(in_data == NULL)
	{
		printk("%s: kmalloc failed\n", __FILE__);
		return -ENOMEM;
	}
	retval = agilent_82357a_receive_bulk_msg(a_priv, in_data, in_data_length, &bytes_read, HZ);
	if(retval)
	{
		printk("%s: %s: agilent_82357a_receive_bulk_msg returned %i, bytes_read=%i\n", __FILE__, __FUNCTION__, retval, bytes_read);
		agilent_82357a_dump_raw_block(in_data, bytes_read);
		kfree(in_data);
		return -EIO;
	}
	if(in_data[0] != (0xff & ~DATA_PIPE_CMD_WR_REGS))
	{
		printk("%s: %s: error, bulk command=0x%x != ~DATA_PIPE_CMD_WR_REGS\n", __FILE__, __FUNCTION__, in_data[0]);
		return -EIO;
	}
	if(in_data[1])
	{
		printk("%s: %s: nonzero error code 0x%x in DATA_PIPE_CMD_WR_REGS response\n", __FILE__, __FUNCTION__, in_data[1]);
		return -EIO;
	}	
	kfree(in_data);
	return 0;
}

int agilent_82357a_read_registers(agilent_82357a_private_t *a_priv, struct agilent_82357a_register_pairlet *reads, 
	int num_reads)
{
	int retval;
	uint8_t *out_data, *in_data;
	int out_data_length, in_data_length;
	int bytes_written, bytes_read;
	int i = 0;
	int j;
	static const int header_length = 2;
	static const int max_reads = 62;
	
	if(num_reads > max_reads)
	{
		printk("%s: %s: bug! num_reads=%i too large\n", __FILE__, __FUNCTION__, num_reads);
	}
	out_data_length = num_reads + header_length;
	out_data = kmalloc(out_data_length, GFP_KERNEL);
	if(out_data == NULL)
	{	
		printk("%s: %s: kmalloc failed\n", __FILE__, __FUNCTION__);
		return -ENOMEM;
	}
	out_data[i++] = DATA_PIPE_CMD_RD_REGS;
	out_data[i++] = num_reads;
	for(j = 0; j < num_reads; j++)
	{	
		out_data[i++] = reads[j].address;
	}	
	if(i > out_data_length)
	{
		printk("%s: bug! buffer overrun\n", __FUNCTION__);
	}
	retval = agilent_82357a_send_bulk_msg(a_priv, out_data, i, &bytes_written, HZ);
	kfree(out_data);
	if(retval)
	{
		printk("%s: %s: agilent_82357a_send_bulk_msg returned %i, bytes_written=%i, i=%i\n", __FILE__, __FUNCTION__, 
			retval, bytes_written, i);		
		return retval;
	}
	in_data_length = 0x20;
	in_data = kmalloc(in_data_length, GFP_KERNEL);
	if(in_data == NULL)
	{
		printk("%s: kmalloc failed\n", __FILE__);
		return -ENOMEM;
	}
	retval = agilent_82357a_receive_bulk_msg(a_priv, in_data, in_data_length, &bytes_read, HZ);
	if(retval)
	{
		printk("%s: %s: agilent_82357a_receive_bulk_msg returned %i, bytes_read=%i\n", __FILE__, __FUNCTION__, retval, bytes_read);
		agilent_82357a_dump_raw_block(in_data, bytes_read);
		kfree(in_data);
		return -EIO;
	}
	i = 0;
	if(in_data[i++] != (0xff & ~DATA_PIPE_CMD_RD_REGS))
	{
		printk("%s: %s: error, bulk command=0x%x != ~DATA_PIPE_CMD_WR_REGS\n", __FILE__, __FUNCTION__, in_data[0]);
		return -EIO;
	}
	if(in_data[i++])
	{
		printk("%s: %s: nonzero error code 0x%x in DATA_PIPE_CMD_RD_REGS response\n", __FILE__, __FUNCTION__, in_data[1]);
		return -EIO;
	}	
	for(j = 0; j < num_reads; j++)
	{
		reads[j].value = in_data[i++];
	}
	kfree(in_data);
	return 0;
}

// interface functions
ssize_t agilent_82357a_read(gpib_board_t *board, uint8_t *buffer, size_t length, int *end, int *nbytes)
{
	int retval;
	agilent_82357a_private_t *a_priv = board->private_data;
	uint8_t *out_data, *in_data;
	int out_data_length, in_data_length;
	int bytes_written, bytes_read;
	int i = 0;
	
	*nbytes = 0;
	out_data_length = 0x9;
	out_data = kmalloc(out_data_length, GFP_KERNEL);
	if(out_data == NULL) return -ENOMEM;
	out_data[i++] = DATA_PIPE_CMD_READ;
	out_data[i++] = 0;	//primary address when ARF_NO_ADDR is not set
	out_data[i++] = 0;	//secondary address when ARF_NO_ADDR is not set
	out_data[i] = ARF_NO_ADDRESS | ARF_END_ON_EOI;
	if(a_priv->eos_mode & REOS)
		out_data[i] |= ARF_END_ON_EOS_CHAR;
	++i;
	out_data[i++] = length & 0xff;
	out_data[i++] = (length >> 8) & 0xff;
	out_data[i++] = (length >> 16) & 0xff;
	out_data[i++] = (length >> 24) & 0xff;
	out_data[i++] = a_priv->eos_char;
		retval = agilent_82357a_send_bulk_msg(a_priv, out_data, i, &bytes_written, HZ);
	kfree(out_data);
	if(retval || bytes_written != i)
	{
		printk("%s: agilent_82357a_send_bulk_msg returned %i, bytes_written=%i, i=%i\n", __FILE__, retval, bytes_written, i);
		return -EIO;
	}
	in_data_length = length;
	in_data = kmalloc(in_data_length, GFP_KERNEL);
	if(in_data == NULL) return -ENOMEM;
	retval = agilent_82357a_receive_bulk_msg(a_priv, in_data, in_data_length, 
		&bytes_read, agilent_82357a_timeout_jiffies(board->usec_timeout));
	if(retval)
	{
		printk("%s: %s: agilent_82357a_receive_bulk_msg returned %i, bytes_read=%i\n", __FILE__, __FUNCTION__, retval, bytes_read);		
		kfree(in_data);
		return -EIO;
	}
	// FIXME
	memcpy(buffer, in_data, length);
	printk("%s: %s: received response:\n", __FILE__, __FUNCTION__);
	agilent_82357a_dump_raw_block(in_data, in_data_length);
	kfree(in_data);
	if(1 /*FIXME*/) *end = 1;
	else *end = 0;
	*nbytes = bytes_read;
	return 0;
}

static ssize_t agilent_82357a_generic_write(gpib_board_t *board, uint8_t *buffer, size_t length, int send_commands, int send_eoi)
{
	int retval;
	agilent_82357a_private_t *a_priv = board->private_data;
	uint8_t *out_data;
	uint8_t status_data[0x6];;
	int out_data_length;	
	int bytes_written;
	int i = 0, j;
	unsigned int bytes_completed;
	
	out_data_length = length + 0x8;
	out_data = kmalloc(out_data_length, GFP_KERNEL);
	if(out_data == NULL) return -ENOMEM;
	out_data[i++] = DATA_PIPE_CMD_WRITE;
	out_data[i++] = 0; // primary address when AWF_NO_ADDRESS is not set
	out_data[i++] = 0; // secondary address when AWF_NO_ADDRESS is not set
	out_data[i] = AWF_NO_ADDRESS | AWF_NO_FAST_TALKER_FIRST_BYTE;
	if(send_commands)
		out_data[i] |= AWF_ATN | AWF_NO_FAST_TALKER;	
	if(send_eoi)
		out_data[i] |= AWF_SEND_EOI;	
	++i;
	out_data[i++] = length & 0xff;
	out_data[i++] = (length >> 8) & 0xff;
	out_data[i++] = (length >> 16) & 0xff;
	out_data[i++] = (length >> 24) & 0xff;
	for(j = 0; j < length; j++)
		out_data[i++] = buffer[j];
	retval = agilent_82357a_send_bulk_msg(a_priv, out_data, i, &bytes_written, 
		agilent_82357a_timeout_jiffies(board->usec_timeout));
	kfree(out_data);
	if(retval || bytes_written != i)
	{
		printk("%s: agilent_82357a_send_bulk_msg returned %i, bytes_written=%i, i=%i\n", __FILE__, retval, bytes_written, i);		
		return -EIO;
	}
	if(wait_event_interruptible(board->wait, test_bit(AIF_WRITE_COMPLETE_BN, &a_priv->interrupt_flags)))
	{
		printk("%s: %s: wait interrupted\n", __FILE__, __FUNCTION__);
	}
	retval = agilent_82357a_receive_control_msg(a_priv, agilent_82357a_control_request, USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE, 
		XFER_STATUS, 0, status_data, sizeof(status_data), HZ);
	if(retval)
	{
		printk("%s: %s: agilent_82357a_receive_control_msg() returned %i\n", __FILE__, __FUNCTION__, retval);
	}
	bytes_completed = status_data[2];
	bytes_completed |= status_data[3] << 8;
	bytes_completed |= status_data[4] << 16;
	bytes_completed |= status_data[5] << 24;
	return bytes_completed;
}

static ssize_t agilent_82357a_write(gpib_board_t *board, uint8_t *buffer, size_t length, int send_eoi)
{
	return agilent_82357a_generic_write(board, buffer, length, 0, send_eoi);
}

ssize_t agilent_82357a_command(gpib_board_t *board, uint8_t *buffer, size_t length)
{
	return agilent_82357a_generic_write(board, buffer, length, 1, 0);
}

int agilent_82357a_take_control(gpib_board_t *board, int synchronous)
{
	agilent_82357a_private_t *a_priv = board->private_data;
	a_priv->bogus_ibsta |= ATN;
	//FIXME: implement
	return 0;
}

int agilent_82357a_go_to_standby(gpib_board_t *board)
{
	agilent_82357a_private_t *a_priv = board->private_data;
	a_priv->bogus_ibsta &= ~ATN;
	//FIXME: implement
	return 0;
}

//FIXME should change prototype to return int
void agilent_82357a_request_system_control(gpib_board_t *board, int request_control)
{
	agilent_82357a_private_t *a_priv = board->private_data;
	struct agilent_82357a_register_pairlet write;
	int retval;
	
	if(request_control)
		a_priv->hw_control_bits |= SYSTEM_CONTROLLER;
	else
		a_priv->hw_control_bits &= ~SYSTEM_CONTROLLER;
	write.address = HW_CONTROL;
	write.value = a_priv->hw_control_bits;
	retval = agilent_82357a_write_registers(a_priv, &write, 1);
	if(retval)
	{
		printk("%s: %s: agilent_82357a_write_registers() returned error\n", __FILE__, __FUNCTION__);
	}
	a_priv->bogus_ibsta |= CIC;
	return;// retval;
}
//FIXME maybe the interface should have a "pulse interface clear" function that can return an error?
void agilent_82357a_interface_clear(gpib_board_t *board, int assert)
{
	//FIXME: implement
	return;
}
void agilent_82357a_remote_enable(gpib_board_t *board, int enable)
{
	//FIXME: implement
	return;// 0;
}
//FIXME need to be able to return error
void agilent_82357a_enable_eos(gpib_board_t *board, uint8_t eos_byte, int compare_8_bits)
{
	agilent_82357a_private_t *a_priv = board->private_data;
	
	a_priv->eos_char = eos_byte;
	a_priv->eos_mode = REOS | BIN;
	if(compare_8_bits == 0)
	{
		printk("%s: %s: warning: hardware only supports 8-bit EOS compare", __FILE__, __FUNCTION__);
	}
}
void agilent_82357a_disable_eos(gpib_board_t *board)
{
	agilent_82357a_private_t *a_priv = board->private_data;
	
	a_priv->eos_mode &= ~REOS;
}
unsigned int agilent_82357a_update_status( gpib_board_t *board, unsigned int clear_mask )
{
	agilent_82357a_private_t *a_priv = board->private_data;
	//FIXME: implement
	return a_priv->bogus_ibsta;
}
//FIXME: prototype should return int
void agilent_82357a_primary_address(gpib_board_t *board, unsigned int address)
{
	//FIXME: implement
}

void agilent_82357a_secondary_address(gpib_board_t *board, unsigned int address, int enable)
{
	//FIXME: implement
	return; // 0;
}
int agilent_82357a_parallel_poll(gpib_board_t *board, uint8_t *result)
{
	//FIXME: implement
	return 0;
}
void agilent_82357a_parallel_poll_configure(gpib_board_t *board, uint8_t config)
{
	//FIXME: implement
	return;// 0;
}
void agilent_82357a_parallel_poll_response(gpib_board_t *board, int ist)
{
	//FIXME: implement
	return;// 0;
}
void agilent_82357a_serial_poll_response(gpib_board_t *board, uint8_t status)
{
	//FIXME: implement
	return;// 0;
}
uint8_t agilent_82357a_serial_poll_status( gpib_board_t *board )
{
	return 0;
}
void agilent_82357a_return_to_local( gpib_board_t *board )
{
	//FIXME: implement
	return;// 0;
}
int agilent_82357a_line_status( const gpib_board_t *board )
{
	//FIXME: implement
	return 0;
}
unsigned int agilent_82357a_t1_delay( gpib_board_t *board, unsigned int nano_sec )
{
	//FIXME: implement
	return 0;
}

void agilent_82357a_interrupt_complete(struct urb *urb, struct pt_regs *regs)
{
	gpib_board_t *board = urb->context;
	agilent_82357a_private_t *a_priv = board->private_data;
	int retval;
	int i;
	uint8_t *transfer_buffer = urb->transfer_buffer;
	unsigned long interrupt_flags;
	
	printk("debug: %s: %s: status=0x%x, error_count=%i, actual_length=%i transfer_buffer:\n", __FILE__, __FUNCTION__,
		urb->status, urb->error_count, urb->actual_length); 
	for(i = 0; i < urb->actual_length; i++)
	{
		printk("%2x ", transfer_buffer[i]); 
	}
	printk("\n");
	// don't resubmit if urb was unlinked
	if(urb->status) return;
	interrupt_flags = transfer_buffer[0];
	if(test_bit(AIF_READ_COMPLETE_BN, &interrupt_flags))
		set_bit(AIF_READ_COMPLETE_BN, &a_priv->interrupt_flags);
	if(test_bit(AIF_WRITE_COMPLETE_BN, &interrupt_flags))
		set_bit(AIF_WRITE_COMPLETE_BN, &a_priv->interrupt_flags);
	if(test_bit(AIF_SRQ_BN, &interrupt_flags))
		set_bit(AIF_SRQ_BN, &a_priv->interrupt_flags);
	retval = usb_submit_urb(a_priv->interrupt_urb, GFP_ATOMIC);
	if(retval)
	{
		printk("%s: failed to resubmit interrupt urb\n", __FUNCTION__);
	}
	wake_up_interruptible(&board->wait);
}

static int agilent_82357a_setup_urbs(gpib_board_t *board)
{
	agilent_82357a_private_t *a_priv = board->private_data;
	struct usb_device *usb_dev;
	int int_pipe;
	int retval;
	
	retval = down_interruptible(&a_priv->interrupt_transfer_lock);
	if(retval) return retval;
	if(a_priv->bus_interface == NULL)
	{
		up(&a_priv->interrupt_transfer_lock);
		return -ENODEV;
	}
	a_priv->interrupt_urb = usb_alloc_urb(0, GFP_KERNEL);
	if(a_priv->interrupt_urb == NULL)
	{
		up(&a_priv->interrupt_transfer_lock);
		return -ENOMEM;
	}
	usb_dev = interface_to_usbdev(a_priv->bus_interface);
	int_pipe = usb_rcvintpipe(usb_dev, AGILENT_82357A_INTERRUPT_IN_ENDPOINT);
	usb_fill_int_urb(a_priv->interrupt_urb, usb_dev, int_pipe, a_priv->interrupt_buffer, 
		sizeof(a_priv->interrupt_buffer), agilent_82357a_interrupt_complete, board, 1);
	retval = usb_submit_urb(a_priv->interrupt_urb, GFP_KERNEL);
	up(&a_priv->interrupt_transfer_lock);
	if(retval) 
	{
		printk("%s: failed to submit first interrupt urb, retval=%i\n", __FILE__, retval);
		return retval;
	}
	return 0;
}

static void agilent_82357a_cleanup_urbs(agilent_82357a_private_t *a_priv)
{
	if(a_priv && a_priv->interrupt_urb)
	{
		usb_kill_urb(a_priv->interrupt_urb);
	}
};

static int agilent_82357a_allocate_private(gpib_board_t *board)
{
	agilent_82357a_private_t *a_priv;

	board->private_data = kmalloc(sizeof(agilent_82357a_private_t), GFP_KERNEL);
	if(board->private_data == NULL)
		return -ENOMEM;
	a_priv = board->private_data;
	memset(a_priv, 0, sizeof(agilent_82357a_private_t));
	init_MUTEX(&a_priv->bulk_transfer_lock);
	init_MUTEX(&a_priv->control_transfer_lock);
	init_MUTEX(&a_priv->interrupt_transfer_lock);
	return 0;
}

static void agilent_82357a_free_private(agilent_82357a_private_t *a_priv)
{
	if(a_priv->interrupt_urb)
		usb_free_urb(a_priv->interrupt_urb);
	kfree(a_priv);
	return;
}

static int agilent_82357a_init(gpib_board_t *board)
{
	agilent_82357a_private_t *a_priv = board->private_data;
	struct agilent_82357a_register_pairlet hw_control;
	struct agilent_82357a_register_pairlet writes[0x2];
	int retval;
	int i = 0;
	
	writes[i].address = PROTOCOL_CONTROL;
	writes[i].value = WRITE_COMPLETE_INTERRUPT_EN;
	i++;
	writes[i].address = LED_CONTROL;
	writes[i].value = FIRMWARE_LED_CONTROL;
	i++;
	if(i > sizeof(writes))
	{
		printk("%s: %s: bug! writes[] overflow\n", __FILE__, __FUNCTION__);
	}
	retval = agilent_82357a_write_registers(a_priv, writes, i);
	if(retval)
	{
		printk("%s: %s: agilent_82357a_write_registers() returned error\n", __FILE__, __FUNCTION__);
	}
	hw_control.address = HW_CONTROL;
	retval = agilent_82357a_read_registers(a_priv, &hw_control, 1);
	if(retval)
	{
		printk("%s: %s: agilent_82357a_read_registers() returned error\n", __FILE__, __FUNCTION__);
	}
	a_priv->hw_control_bits = hw_control.value;
	//FIXME: implement
	return 0;
}
int agilent_82357a_attach(gpib_board_t *board)
{
	int retval;
	int i;
	agilent_82357a_private_t *a_priv;
		
	if(down_interruptible(&agilent_82357a_hotplug_lock))
		return -ERESTARTSYS;
	retval = agilent_82357a_allocate_private(board);
	if(retval < 0)
	{
		up(&agilent_82357a_hotplug_lock);
		return retval;
	}
	a_priv = board->private_data;
	/*FIXME: should allow user to specifiy which device he wants to attach.
	 Use usb_make_path() */
	for(i = 0; i < MAX_NUM_82357A_INTERFACES; i++)
	{
		if(agilent_82357a_driver_interfaces[i] && usb_get_intfdata(agilent_82357a_driver_interfaces[i]) == NULL)
		{
			a_priv->bus_interface = agilent_82357a_driver_interfaces[i];
			usb_set_intfdata(agilent_82357a_driver_interfaces[i], board);
			printk("attached to bus interface %i, address 0x%p\n", i, a_priv->bus_interface);
			break;
		}
	}
	if(i == MAX_NUM_82357A_INTERFACES)
	{
		up(&agilent_82357a_hotplug_lock);
		printk("No NI usb-b gpib adapters found, have you loaded its firmware?\n");
		return -ENODEV;
	}
	retval = agilent_82357a_setup_urbs(board);
	if(retval < 0) 
	{
		up(&agilent_82357a_hotplug_lock);
		return retval;
	}
	retval = agilent_82357a_init(board);
	if(retval < 0) 
	{
		up(&agilent_82357a_hotplug_lock);
		return retval;
	}
	up(&agilent_82357a_hotplug_lock);
	return retval;
}

void agilent_82357a_detach(gpib_board_t *board)
{
	agilent_82357a_private_t *a_priv;
	
	down(&agilent_82357a_hotplug_lock);
//	printk("%s: enter\n", __FUNCTION__);
	a_priv = board->private_data;
	if(a_priv)
	{
		if(a_priv->bus_interface)
		{
			usb_set_intfdata(a_priv->bus_interface, NULL);
		}
		down(&a_priv->bulk_transfer_lock);
		down(&a_priv->control_transfer_lock);
		down(&a_priv->interrupt_transfer_lock);
		
		agilent_82357a_cleanup_urbs(a_priv);
		agilent_82357a_free_private(a_priv);
		
		up(&a_priv->interrupt_transfer_lock);
		up(&a_priv->control_transfer_lock);
		up(&a_priv->bulk_transfer_lock);
	}
//	printk("%s: exit\n", __FUNCTION__);
	up(&agilent_82357a_hotplug_lock);
}

gpib_interface_t agilent_82357a_gpib_interface =
{
	name: "agilent_82357a",
	attach: agilent_82357a_attach,
	detach: agilent_82357a_detach,
	read: agilent_82357a_read,
	write: agilent_82357a_write,
	command: agilent_82357a_command,
	take_control: agilent_82357a_take_control,
	go_to_standby: agilent_82357a_go_to_standby,
	request_system_control: agilent_82357a_request_system_control,
	interface_clear: agilent_82357a_interface_clear,
	remote_enable: agilent_82357a_remote_enable,
	enable_eos: agilent_82357a_enable_eos,
	disable_eos: agilent_82357a_disable_eos,
	parallel_poll: agilent_82357a_parallel_poll,
	parallel_poll_configure: agilent_82357a_parallel_poll_configure,
	parallel_poll_response: agilent_82357a_parallel_poll_response,
	line_status: agilent_82357a_line_status,
	update_status: agilent_82357a_update_status,
	primary_address: agilent_82357a_primary_address,
	secondary_address: agilent_82357a_secondary_address,
	serial_poll_response: agilent_82357a_serial_poll_response,
	serial_poll_status: agilent_82357a_serial_poll_status,
	t1_delay: agilent_82357a_t1_delay,
	return_to_local: agilent_82357a_return_to_local,
	provider_module: &__this_module,
};

// Table with the USB-devices: just now only testing IDs
static struct usb_device_id agilent_82357a_driver_device_table [] = 
{
	{USB_DEVICE(USB_VENDOR_ID_AGILENT, USB_DEVICE_ID_AGILENT_82357A)},
	{} /* Terminating entry */
};


MODULE_DEVICE_TABLE(usb, agilent_82357a_driver_device_table);

static int agilent_82357a_driver_probe(struct usb_interface *interface, 
	const struct usb_device_id *id) 
{
	int i;
	char *path;
	static const int pathLength = 1024;
	
//	printk("agilent_82357a_driver_probe\n");
	if(down_interruptible(&agilent_82357a_hotplug_lock))
		return -ERESTARTSYS;
	usb_get_dev(interface_to_usbdev(interface));
	for(i = 0; i < MAX_NUM_82357A_INTERFACES; i++)
	{
		if(agilent_82357a_driver_interfaces[i] == NULL)
		{
			agilent_82357a_driver_interfaces[i] = interface;
			usb_set_intfdata(interface, NULL);
//			printk("set bus interface %i to address 0x%p\n", i, interface);	
			break;
		}
	}
	if(i == MAX_NUM_82357A_INTERFACES)
	{
		usb_put_dev(interface_to_usbdev(interface));
		up(&agilent_82357a_hotplug_lock);
		printk("out of space in agilent_82357a_driver_interfaces[]\n");
		return -1;
	}
	path = kmalloc(pathLength, GFP_KERNEL);
	if(path == NULL) 
	{
		usb_put_dev(interface_to_usbdev(interface));
		up(&agilent_82357a_hotplug_lock);
		return -ENOMEM;
	}
	usb_make_path(interface_to_usbdev(interface), path, pathLength);
	printk("probe succeeded for path: %s\n", path);
	kfree(path);
	up(&agilent_82357a_hotplug_lock);
	return 0;
}

static void agilent_82357a_driver_disconnect(struct usb_interface *interface) 
{
	int i;
	
	down(&agilent_82357a_hotplug_lock);
//	printk("%s: enter\n", __FUNCTION__);
	for(i = 0; i < MAX_NUM_82357A_INTERFACES; i++)
	{
		if(agilent_82357a_driver_interfaces[i] == interface)
		{
			gpib_board_t *board = usb_get_intfdata(interface);
			
			if(board)
			{
				agilent_82357a_private_t *a_priv = board->private_data;
				
				down(&a_priv->bulk_transfer_lock);
				down(&a_priv->control_transfer_lock);
				down(&a_priv->interrupt_transfer_lock);
				a_priv->bus_interface = NULL;
				up(&a_priv->interrupt_transfer_lock);
				up(&a_priv->control_transfer_lock);
				up(&a_priv->bulk_transfer_lock);
			}	
//			printk("nulled agilent_82357a_driver_interfaces[%i]\n", i);
			agilent_82357a_driver_interfaces[i] = NULL;
			break;
		}
	}
	if(i == MAX_NUM_82357A_INTERFACES)
	{
		printk("unable to find interface in agilent_82357a_driver_interfaces[]? bug?\n");
	}
	usb_put_dev(interface_to_usbdev(interface));
//	printk("%s: exit\n", __FUNCTION__);
	up(&agilent_82357a_hotplug_lock);
}

static struct usb_driver agilent_82357a_bus_driver = 
{
	.owner = &__this_module,
	.name = "agilent_82357a_gpib",
	.probe = agilent_82357a_driver_probe,
	.disconnect = agilent_82357a_driver_disconnect,
	.id_table = agilent_82357a_driver_device_table,
};

static int agilent_82357a_init_module(void)
{
	int i;
	
	info("agilent_82357a_gpib driver loading");
	for(i = 0; i < MAX_NUM_82357A_INTERFACES; i++)
		agilent_82357a_driver_interfaces[i] = NULL;
	usb_register(&agilent_82357a_bus_driver);
	gpib_register_driver(&agilent_82357a_gpib_interface);

	return 0;
}

static void agilent_82357a_exit_module(void)
{
	info("agilent_82357a_gpib driver unloading");
//	printk("%s: enter\n", __FUNCTION__);
	gpib_unregister_driver(&agilent_82357a_gpib_interface);
	usb_deregister(&agilent_82357a_bus_driver);
//	printk("%s: exit\n", __FUNCTION__);
}

module_init(agilent_82357a_init_module);
module_exit(agilent_82357a_exit_module);

