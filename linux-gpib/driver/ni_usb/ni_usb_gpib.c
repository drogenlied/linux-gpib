/***************************************************************************
                          ni_usb/ni_usb_gpib.c  
                             -------------------
 driver for National Instruments usb to gpib adapters

    begin                : 2004-05-29
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
#include "ni_usb_gpib.h"
#include "gpibP.h"
#include "nec7210.h"
#include "tnt4882_registers.h"

MODULE_LICENSE("GPL");

#define MAX_NUM_NI_USB_INTERFACES 128
static struct usb_interface *ni_usb_driver_interfaces[MAX_NUM_NI_USB_INTERFACES];

int ni_usb_parse_status_block(const uint8_t *buffer, struct ni_usb_status_block *status)
{
	uint16_t count;
	
	status->id = buffer[0];
	status->ibsta = (buffer[1] << 8) | buffer[2];
	status->error_code = buffer[3];
	count = buffer[4] | (buffer[5] << 8);
	count = ~count;
	count++;
	status->count = count;
#if 1
printk("status->id 0x%x\n", status->id);
printk("status->ibsta 0x%x\n", status->ibsta);
printk("status->error_code 0x%x\n", status->error_code);
printk("status->count %i\n", status->count);
#endif	
	return 8;
};

int ni_usb_parse_termination_block(const uint8_t *buffer)
{
	int i = 0;
	
	if(buffer[i++] != NIUSB_TERM_ID ||
		buffer[i++] != 0x0 ||
		buffer[i++] != 0x0 ||
		buffer[i++] != 0x0)
		printk("%s: bug: invalid termination block\n", __FILE__);
	return i;	
};

int parse_board_ibrd_readback(const uint8_t *raw_data, struct ni_usb_status_block *status, 
	uint8_t *parsed_data, int parsed_data_length)
{
	static const int ibrd_data_id = 0x36;
	static const int ibrd_data_block_length = 15;
	static const int ibrd_status_id = 0x38;
	int i = 0;
	int j = 0;
	int k;
	unsigned int adr1_bits;
	struct ni_usb_status_block register_write_status;
		
	while(raw_data[i] == ibrd_data_id)
	{
		if(parsed_data_length < j + ibrd_data_block_length)
		{
			printk("%s: bug: buffer too small\n", __FILE__);
			return -EIO;
		}
		i++;
		for(j = 0; j < ibrd_data_block_length; j++)
		{
			parsed_data[j++] = raw_data[i++];
		}
	}
	i += ni_usb_parse_status_block(&raw_data[i], status);
	if(status->id != ibrd_status_id)
	{
		printk("%s: bug: status->id=%i, != ibrd_status_id\n", __FILE__, status->id);
		return -EIO;
	}
	adr1_bits = raw_data[i++];
	if(raw_data[i++] != status->count % ibrd_data_block_length)
	{
		printk("%s: unexpected data: raw_data[%i]=%i, status->count=%i\n", 
			__FILE__, i - 1, (int)raw_data[i - 1], status->count);
	}
	for(k = 0; k < 2; k++)
		if(raw_data[i++] != 0)
		{
			printk("%s: unexpected data: raw_data[%i]=%i, expected 0\n", 
				__FILE__, i - 1, (int)raw_data[i - 1]);
		}
	i += ni_usb_parse_status_block(&raw_data[i], &register_write_status);
	if(register_write_status.id != NIUSB_REG_WRITE_ID)
	{
		printk("%s: unexpected data: register write status id=0x%x, expected 0x%x\n", 
			__FILE__, register_write_status.id, NIUSB_REG_WRITE_ID);
	}
	if(raw_data[i++] != 2)
	{
		printk("%s: unexpected data: register write count=%i, expected 2\n", 
			__FILE__, (int)raw_data[i - 1]);
	}
	for(k = 0; k < 3; k++)
		if(raw_data[i++] != 0)
		{
			printk("%s: unexpected data: raw_data[%i]=%i, expected 0\n", 
				__FILE__, i - 1, (int)raw_data[i - 1]);
		}
	i += ni_usb_parse_termination_block(&raw_data[i]);
	return i;
}

// interface functions
ssize_t ni_usb_read(gpib_board_t *board, uint8_t *buffer, size_t length, int *end)
{
	int retval;
	ni_usb_private_t *ni_priv = board->private_data;
	struct usb_device *usb_dev;
	int out_pipe, in_pipe;
	uint8_t *out_data, *in_data;
	int out_data_length, in_data_length;
	int bytes_written, bytes_read;
	int i = 0;
	int complement_count;
	struct ni_usb_status_block status;
	int timeout_code = 0xd;	//FIXME
	static const int max_read_length = 0xffff;
	int eos_mode = 0x0; // FIXME
	int eos_char = 0x0; // FIXME
	
	if(length > max_read_length)
	{
		length = max_read_length;
		printk("%s: read length too long\n", __FILE__);
	}
	usb_dev = interface_to_usbdev(ni_priv->bus_interface);
	out_pipe = usb_sndbulkpipe(usb_dev, NIUSB_BULK_OUT_ENDPOINT);
	out_data_length = 0x20;
	out_data = kmalloc(out_data_length, GFP_KERNEL);
	if(out_data == NULL) return -ENOMEM;
	out_data[i++] = 0x0a;
	out_data[i++] = eos_mode >> 8;
	out_data[i++] = eos_char;
	out_data[i++] = 0xf0 | timeout_code;
	complement_count = length;
	complement_count = length - 1;
	complement_count = ~complement_count;
	out_data[i++] = complement_count & 0xff;
	out_data[i++] = (complement_count >> 8) & 0xff;
	out_data[i++] = 0x0;
	out_data[i++] = 0x0;
	i += ni_usb_bulk_register_write_header(&out_data[i], 2);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(AUXMR), AUX_HLDI);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(AUXMR), AUX_CLEAR_END);
	while(i % 4)	// pad with zeros to 4-byte boundary
		out_data[i++] = 0x0;
	i += ni_usb_bulk_termination(&out_data[i]);
	retval = usb_bulk_msg(usb_dev, out_pipe, out_data, i, &bytes_written, HZ /*FIXME set timeout properly */);
	kfree(out_data);
	if(retval || bytes_written != i)
	{
		printk("%s: usb_bulk_msg returned %i, bytes_written=%i, i=%i\n", __FILE__, retval, bytes_written, i);
		return -EIO;
	}
	in_data_length = (length / 15 + 1) * 0x10 + 0x20;
	in_data = kmalloc(in_data_length, GFP_KERNEL);
	if(in_data == NULL) return -ENOMEM;
	in_pipe = usb_rcvbulkpipe(usb_dev, NIUSB_BULK_IN_ENDPOINT);
	retval = usb_bulk_msg(usb_dev, in_pipe, in_data, in_data_length, &bytes_read, HZ /*FIXME set timeout properly */);
	if(retval)
	{
		printk("%s: %s: usb_bulk_msg returned %i, bytes_read=%i\n", __FILE__, __FUNCTION__, retval, bytes_read);		
		kfree(in_data);
		return -EIO;
	}
	parse_board_ibrd_readback(in_data, &status, buffer, length);
	kfree(in_data);
	//FIXME update ibsta using status info
	return status.count;
}

static ssize_t ni_usb_write(gpib_board_t *board, uint8_t *buffer, size_t length, int send_eoi)
{
	int retval;
	ni_usb_private_t *ni_priv = board->private_data;
	struct usb_device *usb_dev;
	int out_pipe, in_pipe;
	uint8_t *out_data, *in_data;
	int out_data_length, in_data_length;
	int bytes_written, bytes_read;
	int i = 0, j;
	int complement_count;
	struct ni_usb_status_block status;
	int timeout_code = 0xd;	//FIXME
	static const int max_write_length = 0xffff;
	
	if(length > max_write_length)
	{
		length = max_write_length;
		send_eoi = 0;
		printk("%s: write length too long\n", __FILE__);
	}
	usb_dev = interface_to_usbdev(ni_priv->bus_interface);
	out_pipe = usb_sndbulkpipe(usb_dev, NIUSB_BULK_OUT_ENDPOINT);
	out_data_length = length + 0x10;
	out_data = kmalloc(out_data_length, GFP_KERNEL);
	if(out_data == NULL) return -ENOMEM;
	out_data[i++] = 0x0d;
	complement_count = length;
	complement_count = length - 1;
	complement_count = ~complement_count;
	out_data[i++] = complement_count & 0xff;
	out_data[i++] = (complement_count >> 8) & 0xff;
	out_data[i++] = 0xf0 | timeout_code;
	out_data[i++] = 0x0;
	out_data[i++] = 0x0;
	if(send_eoi)
		out_data[i++] = 0x8;
	else
		out_data[i++] = 0x0;
	out_data[i++] = 0x0;
	for(j = 0; j < length; j++)
		out_data[i++] = buffer[j];
	while(i % 4)	// pad with zeros to 4-byte boundary
		out_data[i++] = 0x0;
	i += ni_usb_bulk_termination(&out_data[i]);
	retval = usb_bulk_msg(usb_dev, out_pipe, out_data, i, &bytes_written, HZ /*FIXME set timeout properly */);
	kfree(out_data);
	if(retval || bytes_written != i)
	{
		printk("%s: usb_bulk_msg returned %i, bytes_written=%i, i=%i\n", __FILE__, retval, bytes_written, i);		
		return -EIO;
	}
	in_data_length = 0x10;
	in_data = kmalloc(in_data_length, GFP_KERNEL);
	if(in_data == NULL) return -ENOMEM;
	in_pipe = usb_rcvbulkpipe(usb_dev, NIUSB_BULK_IN_ENDPOINT);
	retval = usb_bulk_msg(usb_dev, in_pipe, in_data, in_data_length, &bytes_read, HZ /*FIXME set timeout properly */);
	if(retval || bytes_read != 12)
	{
		printk("%s: %s: usb_bulk_msg returned %i, bytes_read=%i\n", __FILE__, __FUNCTION__, retval, bytes_read);		
		kfree(in_data);
		return -EIO;
	}
	ni_usb_parse_status_block(in_data, &status);
	kfree(in_data);
	//FIXME update ibsta using status info
	return status.count;
}
ssize_t ni_usb_command(gpib_board_t *board, uint8_t *buffer, size_t length)
{
	int retval;
	ni_usb_private_t *ni_priv = board->private_data;
	struct usb_device *usb_dev;
	int out_pipe, in_pipe;
	uint8_t *out_data, *in_data;
	int out_data_length, in_data_length;
	int bytes_written, bytes_read;
	int i = 0, j;
	int complement_count;
	struct ni_usb_status_block status;
	int timeout_code = 0xd;	//FIXME
	static const int max_command_length = 0xff;
	
	if(length > max_command_length) length = max_command_length;
	usb_dev = interface_to_usbdev(ni_priv->bus_interface);
	out_pipe = usb_sndbulkpipe(usb_dev, NIUSB_BULK_OUT_ENDPOINT);
	out_data_length = length + 0x10;
	out_data = kmalloc(out_data_length, GFP_KERNEL);
	if(out_data == NULL) return -ENOMEM;
	out_data[i++] = 0x0c;
	complement_count = length;
	complement_count = length - 1;
	complement_count = ~complement_count;
	out_data[i++] = complement_count;
	out_data[i++] = 0x0;
	out_data[i++] = 0xf0 | timeout_code;
	for(j = 0; j < length; j++)
		out_data[i++] = buffer[j];
	while(i % 4)	// pad with zeros to 4-byte boundary
		out_data[i++] = 0x0;
	i += ni_usb_bulk_termination(&out_data[i]);
	retval = usb_bulk_msg(usb_dev, out_pipe, out_data, i, &bytes_written, HZ /*FIXME set timeout properly */);
	kfree(out_data);
	if(retval || bytes_written != i)
	{
		printk("%s: usb_bulk_msg returned %i, bytes_written=%i, i=%i\n", __FILE__, retval, bytes_written, i);		
		return -EIO;
	}
	in_data_length = 0x10;
	in_data = kmalloc(in_data_length, GFP_KERNEL);
	if(in_data == NULL) return -ENOMEM;
	in_pipe = usb_rcvbulkpipe(usb_dev, NIUSB_BULK_IN_ENDPOINT);
	retval = usb_bulk_msg(usb_dev, in_pipe, in_data, in_data_length, &bytes_read, HZ /*FIXME set timeout properly */);
	if(retval || bytes_read != 12)
	{
		printk("%s: %s: usb_bulk_msg returned %i, bytes_read=%i\n", __FILE__, __FUNCTION__, retval, bytes_read);		
		kfree(in_data);
		return -EIO;
	}
	ni_usb_parse_status_block(in_data, &status);
	kfree(in_data);
	//FIXME update ibsta using status info
	return status.count;
}

int ni_usb_take_control(gpib_board_t *board, int synchronous)
{
	int retval;
	ni_usb_private_t *ni_priv = board->private_data;
	struct usb_device *usb_dev;
	int out_pipe, in_pipe;
	uint8_t *out_data, *in_data;
	int out_data_length, in_data_length;
	int bytes_written, bytes_read;
	int i = 0;
	struct ni_usb_status_block status;
	
	usb_dev = interface_to_usbdev(ni_priv->bus_interface);
	out_pipe = usb_sndbulkpipe(usb_dev, NIUSB_BULK_OUT_ENDPOINT);
	out_data_length = 0x10;
	out_data = kmalloc(out_data_length, GFP_KERNEL);
	if(out_data == NULL)
	{	
		printk("%s: kmalloc failed\n", __FILE__);
		return -ENOMEM;
	}
	out_data[i++] = NIUSB_IBCAC_ID;
	if(synchronous)
		out_data[i++] = 0x1;
	else
		out_data[i++] = 0x0;
	out_data[i++] = 0x0;
	out_data[i++] = 0x0;
	i += ni_usb_bulk_termination(&out_data[i]);
	retval = usb_bulk_msg(usb_dev, out_pipe, out_data, i, &bytes_written, HZ /*FIXME set timeout properly */);
	kfree(out_data);
	if(retval || bytes_written != i)
	{
		printk("%s: usb_bulk_msg returned %i, bytes_written=%i, i=%i\n", __FILE__, retval, bytes_written, i);		
		return retval;
	}
	in_data_length = 0x10;
	in_data = kmalloc(in_data_length, GFP_KERNEL);
	if(in_data == NULL)
	{
		printk("%s: kmalloc failed\n", __FILE__);
		return -ENOMEM;
	}
	in_pipe = usb_rcvbulkpipe(usb_dev, NIUSB_BULK_IN_ENDPOINT);
	retval = usb_bulk_msg(usb_dev, in_pipe, in_data, in_data_length, &bytes_read, HZ /*FIXME set timeout properly */);
	if(retval || bytes_read != 12)
	{
		printk("%s: %s: usb_bulk_msg returned %i, bytes_read=%i\n", __FILE__, __FUNCTION__, retval, bytes_read);		
		kfree(in_data);
		return retval;
	}
	ni_usb_parse_status_block(in_data, &status);
	kfree(in_data);
	//FIXME update ibsta using status info
	return 0;
}
int ni_usb_go_to_standby(gpib_board_t *board)
{
	int retval;
	ni_usb_private_t *ni_priv = board->private_data;
	struct usb_device *usb_dev;
	int out_pipe, in_pipe;
	uint8_t *out_data, *in_data;
	int out_data_length, in_data_length;
	int bytes_written, bytes_read;
	int i = 0;
	struct ni_usb_status_block status;
	
	usb_dev = interface_to_usbdev(ni_priv->bus_interface);
	out_pipe = usb_sndbulkpipe(usb_dev, NIUSB_BULK_OUT_ENDPOINT);
	out_data_length = 0x10;
	out_data = kmalloc(out_data_length, GFP_KERNEL);
	if(out_data == NULL)
	{	
		printk("%s: kmalloc failed\n", __FILE__);
		return -ENOMEM;
	}
	out_data[i++] = NIUSB_IBGTS_ID;
	out_data[i++] = 0x0;
	out_data[i++] = 0x0;
	out_data[i++] = 0x0;
	i += ni_usb_bulk_termination(&out_data[i]);
	retval = usb_bulk_msg(usb_dev, out_pipe, out_data, i, &bytes_written, HZ /*FIXME set timeout properly */);
	kfree(out_data);
	if(retval || bytes_written != i)
	{
		printk("%s: usb_bulk_msg returned %i, bytes_written=%i, i=%i\n", __FILE__, retval, bytes_written, i);		
		return retval;
	}
	in_data_length = 0x10;
	in_data = kmalloc(in_data_length, GFP_KERNEL);
	if(in_data == NULL)
	{
		printk("%s: kmalloc failed\n", __FILE__);
		return -ENOMEM;
	}
	in_pipe = usb_rcvbulkpipe(usb_dev, NIUSB_BULK_IN_ENDPOINT);
	retval = usb_bulk_msg(usb_dev, in_pipe, in_data, in_data_length, &bytes_read, HZ /*FIXME set timeout properly */);
	if(retval || bytes_read != 12)
	{
		printk("%s: %s: usb_bulk_msg returned %i, bytes_read=%i\n", __FILE__, __FUNCTION__, retval, bytes_read);		
		kfree(in_data);
		return retval;
	}
	ni_usb_parse_status_block(in_data, &status);
	kfree(in_data);
	//FIXME update ibsta using status info
	return 0;
}
//FIXME should change prototype to return int
void ni_usb_request_system_control( gpib_board_t *board, int request_control )
{
	int retval;
	ni_usb_private_t *ni_priv = board->private_data;
	struct usb_device *usb_dev;
	int out_pipe, in_pipe;
	uint8_t *out_data, *in_data;
	int out_data_length, in_data_length;
	int bytes_written, bytes_read;
	int i = 0;
	struct ni_usb_status_block status;
	
	usb_dev = interface_to_usbdev(ni_priv->bus_interface);
	out_pipe = usb_sndbulkpipe(usb_dev, NIUSB_BULK_OUT_ENDPOINT);
	out_data_length = 0x20;
	out_data = kmalloc(out_data_length, GFP_KERNEL);
	if(out_data == NULL)
	{	
		printk("%s: kmalloc failed\n", __FILE__);
		return;// -ENOMEM;
	}
	if(request_control)
	{
		i += ni_usb_bulk_register_write_header(&out_data[i], 2);
		i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, CMDR, SETSC);
		i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(AUXMR), AUX_CIFC);
	}else
	{
		i += ni_usb_bulk_register_write_header(out_data, 4);
		i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(AUXMR), AUX_CREN);
		i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(AUXMR), AUX_CIFC);
		i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(AUXMR), AUX_DSC);
		i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, CMDR, CLRSC);
	}
	while(i % 4)
		out_data[i++] = 0x00;
	i += ni_usb_bulk_termination(&out_data[i]);
	retval = usb_bulk_msg(usb_dev, out_pipe, out_data, i, &bytes_written, HZ /*FIXME set timeout properly */);
	kfree(out_data);
	if(retval || bytes_written != i)
	{
		printk("%s: usb_bulk_msg returned %i, bytes_written=%i, i=%i\n", __FILE__, retval, bytes_written, i);		
		return;// retval;
	}
	in_data_length = 0x20;
	in_data = kmalloc(in_data_length, GFP_KERNEL);
	if(in_data == NULL)
	{
		printk("%s: kmalloc failed\n", __FILE__);
		return;// -ENOMEM;
	}
	in_pipe = usb_rcvbulkpipe(usb_dev, NIUSB_BULK_IN_ENDPOINT);
	retval = usb_bulk_msg(usb_dev, in_pipe, in_data, in_data_length, &bytes_read, HZ /*FIXME set timeout properly */);
	if(retval || bytes_read != 16)
	{
		printk("%s: %s: usb_bulk_msg returned %i, bytes_read=%i\n", __FILE__, __FUNCTION__, retval, bytes_read);		
		kfree(in_data);
		return;// retval;
	}
	ni_usb_parse_status_block(in_data, &status);
	//FIXME parse extra 09 status bits and termination
	kfree(in_data);
	//FIXME update ibsta using status info
	return;// 0;
}
//FIXME maybe the interface should have a "pulse interface clear" function that can return an error?
void ni_usb_interface_clear(gpib_board_t *board, int assert)
{
	int retval;
	ni_usb_private_t *ni_priv = board->private_data;
	struct usb_device *usb_dev;
	int out_pipe, in_pipe;
	uint8_t *out_data, *in_data;
	int out_data_length, in_data_length;
	int bytes_written, bytes_read;
	int i = 0;
	struct ni_usb_status_block status;
	
	// FIXME: we are going to pulse when assert is true, and ignore otherwise
	if(assert == 0) return;
	usb_dev = interface_to_usbdev(ni_priv->bus_interface);
	out_pipe = usb_sndbulkpipe(usb_dev, NIUSB_BULK_OUT_ENDPOINT);
	out_data_length = 0x10;
	out_data = kmalloc(out_data_length, GFP_KERNEL);
	if(out_data == NULL)
	{	
		printk("%s: kmalloc failed\n", __FILE__);
		return;
	}
	out_data[i++] = NIUSB_IBSIC_ID;
	out_data[i++] = 0x0;
	out_data[i++] = 0x0;
	out_data[i++] = 0x0;
	i += ni_usb_bulk_termination(&out_data[i]);
	retval = usb_bulk_msg(usb_dev, out_pipe, out_data, i, &bytes_written, HZ /*FIXME set timeout properly */);
	kfree(out_data);
	if(retval || bytes_written != i)
	{
		printk("%s: %s: usb_bulk_msg returned %i, bytes_written=%i, i=%i\n", __FILE__, __FUNCTION__, retval, bytes_written, i);
		return;
	}
	in_data_length = 0x10;
	in_data = kmalloc(in_data_length, GFP_KERNEL);
	if(in_data == NULL)
	{
		printk("%s: kmalloc failed\n", __FILE__);
		return;
	}
	in_pipe = usb_rcvbulkpipe(usb_dev, NIUSB_BULK_IN_ENDPOINT);
	retval = usb_bulk_msg(usb_dev, in_pipe, in_data, in_data_length, &bytes_read, HZ /*FIXME set timeout properly */);
	if(retval || bytes_read != 12)
	{
		printk("%s: %s: usb_bulk_msg returned %i, bytes_read=%i\n", __FILE__, __FUNCTION__, retval, bytes_read);		
		kfree(in_data);
		return;
	}
printk("ifc response: ");
for(i=0;i<bytes_read;i++)
{
printk(" 0x%x", (int)in_data[i]);
}	
printk("\n");
	ni_usb_parse_status_block(in_data, &status);
	kfree(in_data);
	//FIXME update ibsta using status info
	return;
}
void ni_usb_remote_enable(gpib_board_t *board, int enable)
{
}
void ni_usb_enable_eos(gpib_board_t *board, uint8_t eos_byte, int compare_8_bits)
{
}
void ni_usb_disable_eos(gpib_board_t *board)
{
}
unsigned int ni_usb_update_status( gpib_board_t *board, unsigned int clear_mask )
{
	int retval;
	ni_usb_private_t *ni_priv = board->private_data;
	struct usb_device *usb_dev;
	int pipe;
	static const int bufferLength = 8;
	uint8_t *buffer;
	struct ni_usb_status_block status;
	static const unsigned int ni_usb_ibsta_mask = SRQI | ATN | CIC | REM | LACS | TACS;
	
	usb_dev = interface_to_usbdev(ni_priv->bus_interface);
	pipe = usb_rcvctrlpipe(usb_dev, 0);
	//printk("%s: receive control pipe is %i\n", __FILE__, pipe);
	buffer = kmalloc(bufferLength, GFP_KERNEL);
	if(buffer == NULL)
	{
		return -ENOMEM;
	}
	retval = usb_control_msg(usb_dev, pipe, 0x21, USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE, 
		0x200, 0x0, buffer, bufferLength, HZ);
	if(retval != bufferLength)
	{
		printk("%s: usb_control_msg returned %i\n", __FILE__, retval);		
		kfree(buffer);
		return -1;
	}
	ni_usb_parse_status_block(buffer, &status);
	kfree(buffer);
	board->status &= ~clear_mask;
	board->status &= ~ni_usb_ibsta_mask;
	board->status |= status.ibsta & ni_usb_ibsta_mask;	
	if(status.ibsta & ~ni_usb_ibsta_mask)
	{
		printk("%s: debug: ibsta from status block is 0x%x\n", __FILE__, status.ibsta);
	}
	return board->status;
}
//FIXME: prototype should return int
void ni_usb_primary_address(gpib_board_t *board, unsigned int address)
{
	int retval;
	ni_usb_private_t *ni_priv = board->private_data;
	struct usb_device *usb_dev;
	int out_pipe, in_pipe;
	uint8_t *out_data, *in_data;
	int out_data_length, in_data_length;
	int bytes_written, bytes_read;
	int i = 0;
	struct ni_usb_status_block status;
	
	usb_dev = interface_to_usbdev(ni_priv->bus_interface);
	out_pipe = usb_sndbulkpipe(usb_dev, NIUSB_BULK_OUT_ENDPOINT);
	out_data_length = 0x20;
	out_data = kmalloc(out_data_length, GFP_KERNEL);
	if(out_data == NULL)
	{	
		printk("%s: kmalloc failed\n", __FILE__);
		return;// -ENOMEM;
	}
	i += ni_usb_bulk_register_write_header(&out_data[i], 2);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(ADR), address);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_UNKNOWN2, 0x0, address);
	while(i % 4)
		out_data[i++] = 0x0;
	i += ni_usb_bulk_termination(&out_data[i]);
	retval = usb_bulk_msg(usb_dev, out_pipe, out_data, i, &bytes_written, HZ /*FIXME set timeout properly */);
	kfree(out_data);
	if(retval || bytes_written != i)
	{
		printk("%s: usb_bulk_msg returned %i, bytes_written=%i, i=%i\n", __FILE__, retval, bytes_written, i);		
		return;// retval;
	}
	in_data_length = 0x20;
	in_data = kmalloc(in_data_length, GFP_KERNEL);
	if(in_data == NULL)
	{
		printk("%s: kmalloc failed\n", __FILE__);
		return;// -ENOMEM;
	}
	in_pipe = usb_rcvbulkpipe(usb_dev, NIUSB_BULK_IN_ENDPOINT);
	retval = usb_bulk_msg(usb_dev, in_pipe, in_data, in_data_length, &bytes_read, HZ /*FIXME set timeout properly */);
	if(retval || bytes_read != 16)
	{
		printk("%s: %s: usb_bulk_msg returned %i, bytes_read=%i\n", __FILE__, __FUNCTION__, retval, bytes_read);		
		kfree(in_data);
		return;// retval;
	}
	ni_usb_parse_status_block(in_data, &status);
	kfree(in_data);
	//FIXME update ibsta using status info
	return;// 0;
}
void ni_usb_secondary_address(gpib_board_t *board, unsigned int address, int enable)
{
	int retval;
	ni_usb_private_t *ni_priv = board->private_data;
	struct usb_device *usb_dev;
	int out_pipe, in_pipe;
	uint8_t *out_data, *in_data;
	int out_data_length, in_data_length;
	int bytes_written, bytes_read;
	int i = 0;
	struct ni_usb_status_block status;
	unsigned int adr_bits, admr_bits;
		
	usb_dev = interface_to_usbdev(ni_priv->bus_interface);
	out_pipe = usb_sndbulkpipe(usb_dev, NIUSB_BULK_OUT_ENDPOINT);
	out_data_length = 0x20;
	out_data = kmalloc(out_data_length, GFP_KERNEL);
	if(out_data == NULL)
	{	
		printk("%s: kmalloc failed\n", __FILE__);
		return;// -ENOMEM;
	}
	i += ni_usb_bulk_register_write_header(&out_data[i], 3);
	adr_bits = HR_ARS;
	admr_bits = HR_TRM0 | HR_TRM1;
	if(enable)
	{
		adr_bits |= address;
		admr_bits |= HR_ADM1;
	}else
	{
		adr_bits |= HR_DT | HR_DL;
		admr_bits |= HR_ADM0;
	}
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(ADR), adr_bits);

	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(ADMR), admr_bits);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_UNKNOWN2, 0x1, enable ? MSA(address) : 0x0);
	while(i % 4)
		out_data[i++] = 0x0;
	i += ni_usb_bulk_termination(&out_data[i]);
	retval = usb_bulk_msg(usb_dev, out_pipe, out_data, i, &bytes_written, HZ /*FIXME set timeout properly */);
	kfree(out_data);
	if(retval || bytes_written != i)
	{
		printk("%s: usb_bulk_msg returned %i, bytes_written=%i, i=%i\n", __FILE__, retval, bytes_written, i);		
		return; // retval;
	}
	in_data_length = 0x20;
	in_data = kmalloc(in_data_length, GFP_KERNEL);
	if(in_data == NULL)
	{
		printk("%s: kmalloc failed\n", __FILE__);
		return; // -ENOMEM;
	}
	in_pipe = usb_rcvbulkpipe(usb_dev, NIUSB_BULK_IN_ENDPOINT);
	retval = usb_bulk_msg(usb_dev, in_pipe, in_data, in_data_length, &bytes_read, HZ /*FIXME set timeout properly */);
	if(retval || bytes_read != 16)
	{
		printk("%s: %s: usb_bulk_msg returned %i, bytes_read=%i\n", __FILE__, __FUNCTION__, retval, bytes_read);		
		kfree(in_data);
		return; // retval;
	}
	ni_usb_parse_status_block(in_data, &status);
	kfree(in_data);
	//FIXME update ibsta using status info
	return; // 0;
}
int ni_usb_parallel_poll(gpib_board_t *board, uint8_t *result)
{
	return 0;
}
void ni_usb_parallel_poll_configure(gpib_board_t *board, uint8_t config )
{
}
void ni_usb_parallel_poll_response(gpib_board_t *board, int ist )
{
}
void ni_usb_serial_poll_response(gpib_board_t *board, uint8_t status)
{
}
uint8_t ni_usb_serial_poll_status( gpib_board_t *board )
{
	return 0;
}
void ni_usb_return_to_local( gpib_board_t *board )
{
}
int ni_usb_line_status( const gpib_board_t *board )
{
	return 0;
}
unsigned int ni_usb_t1_delay( gpib_board_t *board, unsigned int nano_sec )
{
	return 0;
}
int ni_usb_allocate_private(gpib_board_t *board)
{
	ni_usb_private_t *ni_priv;

	board->private_data = kmalloc(sizeof(ni_usb_private_t), GFP_KERNEL);
	if(board->private_data == NULL)
		return -ENOMEM;
	ni_priv = board->private_data;
	memset(ni_priv, 0, sizeof(ni_usb_private_t));
	return 0;
}

int ni_usb_init(gpib_board_t *board)
{
	int retval;
	ni_usb_private_t *ni_priv = board->private_data;
	struct usb_device *usb_dev;
	int out_pipe, in_pipe;
	uint8_t *out_data, *in_data;
	int out_data_length, in_data_length;
	int bytes_written, bytes_read;
	int i = 0, j;
	struct ni_usb_status_block status;
	
/*
on startup,
windows driver writes: 08 04 03 08 03 09 03 0a 03 0b 00 00 04 00 00 00
then reads back: 34 01 0a c3 34 e2 0a c3 35 01 00 00 04 00 00 00
*/
	usb_dev = interface_to_usbdev(ni_priv->bus_interface);
	out_pipe = usb_sndbulkpipe(usb_dev, NIUSB_BULK_OUT_ENDPOINT);
	out_data_length = 0x100;
	out_data = kmalloc(out_data_length, GFP_KERNEL);
	if(out_data == NULL)
	{	
		printk("%s: kmalloc failed\n", __FILE__);
		return -ENOMEM;
	}
	i += ni_usb_bulk_register_write_header(&out_data[i], 26);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_UNKNOWN3, 0x10, 0x0);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, CMDR, SOFT_RESET);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(AUXMR), AUX_7210);
	// don't know why NI writes 0x99 to the SPMR
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(SPMR), 0x99);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, HSSEL, TNT_ONE_CHIP_BIT);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(AUXMR), AUX_CR);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, IMR0, TNT_IMR0_ALWAYS_BITS);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(IMR1), 0x0);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(IMR2), 0x0);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, IMR3, 0x0);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(AUXMR), AUX_HLDI);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(AUXMR), AUXRI | SISB | USTD);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(AUXMR), AUXRB | HR_TRI);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, KEYREG, 0x0);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(AUXMR), AUXRG | NTNL_BIT);
	// request system control
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, CMDR, SETSC);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(AUXMR), AUX_CIFC);
	// primary address 0
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(ADR), 0x0);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_UNKNOWN2, 0x0, 0x0);
	// secondary address disabled
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(ADR), HR_ARS | HR_DT | HR_DL);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(ADMR), HR_TRM0 | HR_TRM1 | HR_ADM0);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_UNKNOWN2, 0x1, 0x0);
	// is this a timeout?
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_UNKNOWN2, 0x2, 0xfd);
	// what is this?  There is no documented tnt4882 register at offset 0xf
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, 0xf, 0x11);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(AUXMR), AUX_PON);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(AUXMR), AUX_CPPF);		
	while(i % 4)
		out_data[i++] = 0x0;
#if 0
	out_data[i++] = NIUSB_UNKNOWN3_ID;
	out_data[i++] = 0x1; // don't know what this means
	out_data[i++] = 0x0;
	out_data[i++] = 0x0;
	i += ni_usb_bulk_register_write_header(&out_data[i], 1);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_UNKNOWN2, 0x3, 0x1);
	while(i % 4)
		out_data[i++] = 0x0;
		i += ni_usb_bulk_register_write_header(&out_data[i], 4);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_UNKNOWN2, 0x4, 0x1);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_UNKNOWN2, 0x5, 0x1);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_UNKNOWN2, 0x6, 0x0);
	i += ni_usb_bulk_register_write(&out_data[i], NIUSB_SUBDEV_UNKNOWN2, 0x7, 0xfb);
	while(i % 4)
		out_data[i++] = 0x0;
#endif
	i += ni_usb_bulk_termination(&out_data[i]);
	retval = usb_bulk_msg(usb_dev, out_pipe, out_data, i, &bytes_written, HZ /*FIXME set timeout properly */);
printk("out_data:\n");
for(j=0;j<i;j++)	
{
	printk("%.2x ", (int)out_data[j]);
}
printk("\n");
	kfree(out_data);
	if(retval || bytes_written != i)
	{
		printk("%s: usb_bulk_msg returned %i, bytes_written=%i, i=%i\n", __FILE__, retval, bytes_written, i);		
		return retval;
	}
	in_data_length = 0x30;
	in_data = kmalloc(in_data_length, GFP_KERNEL);
	if(in_data == NULL)
	{
		printk("%s: kmalloc failed\n", __FILE__);
		return -ENOMEM;
	}
	in_pipe = usb_rcvbulkpipe(usb_dev, NIUSB_BULK_IN_ENDPOINT);
	retval = usb_bulk_msg(usb_dev, in_pipe, in_data, in_data_length, &bytes_read, HZ /*FIXME set timeout properly */);
	if(retval || bytes_read != 0x10)
	{
		printk("%s: %s: usb_bulk_msg returned %i, bytes_read=%i\n", __FILE__, __FUNCTION__, retval, bytes_read);		
		kfree(in_data);
		return retval;
	}
	ni_usb_parse_status_block(in_data, &status);
	kfree(in_data);
	//FIXME update ibsta using status info
	return 0;
}

int ni_usb_attach(gpib_board_t *board)
{
	int retval;
	int i;
	ni_usb_private_t *ni_priv;
		
	retval = ni_usb_allocate_private(board);
	if(retval < 0) return retval;
	ni_priv = board->private_data;
	/*FIXME: should allow user to specifiy which device he wants to attach.
	 Use usb_make_path() */
	for(i = 0; i < MAX_NUM_NI_USB_INTERFACES; i++)
	{
		if(ni_usb_driver_interfaces[i])
		{
			ni_priv->bus_interface = ni_usb_driver_interfaces[i];
			break;
		}
	}
	if(i == MAX_NUM_NI_USB_INTERFACES)
	{
		printk("no NI usb-b gpib adapters found\n");
		return -1;
	}

	return ni_usb_init(board);
}

void ni_usb_detach(gpib_board_t *board)
{
}

gpib_interface_t ni_usb_gpib_interface =
{
	name: "ni_usb_b",
	attach: ni_usb_attach,
	detach: ni_usb_detach,
	read: ni_usb_read,
	write: ni_usb_write,
	command: ni_usb_command,
	take_control: ni_usb_take_control,
	go_to_standby: ni_usb_go_to_standby,
	request_system_control: ni_usb_request_system_control,
	interface_clear: ni_usb_interface_clear,
	remote_enable: ni_usb_remote_enable,
	enable_eos: ni_usb_enable_eos,
	disable_eos: ni_usb_disable_eos,
	parallel_poll: ni_usb_parallel_poll,
	parallel_poll_configure: ni_usb_parallel_poll_configure,
	parallel_poll_response: ni_usb_parallel_poll_response,
	line_status: ni_usb_line_status,
	update_status: ni_usb_update_status,
	primary_address: ni_usb_primary_address,
	secondary_address: ni_usb_secondary_address,
	serial_poll_response: ni_usb_serial_poll_response,
	serial_poll_status: ni_usb_serial_poll_status,
	t1_delay: ni_usb_t1_delay,
	return_to_local: ni_usb_return_to_local,
	provider_module: &__this_module,
};

// Table with the USB-devices: just now only testing IDs
static struct usb_device_id ni_usb_driver_device_table [] = 
{
	{ 
		USB_DEVICE(USB_VENDOR_ID_NI, USB_DEVICE_ID_NI_USB_B),
	},
	{} /* Terminating entry */
};


MODULE_DEVICE_TABLE(usb, ni_usb_driver_device_table);

static int ni_usb_driver_probe(struct usb_interface *interface, 
	const struct usb_device_id *id) 
{
	int i;
	
	printk("ni_usb_driver_probe\n");
	for(i = 0; i < MAX_NUM_NI_USB_INTERFACES; i++)
	{
		if(ni_usb_driver_interfaces[i] == NULL)
		{
			ni_usb_driver_interfaces[i] = interface;
			break;
		}
	}
	if(i == MAX_NUM_NI_USB_INTERFACES)
	{
		printk("out of space in ni_usb_driver_interfaces[]\n");
		return -1;
	}
	return 0;
}

static void ni_usb_driver_disconnect(struct usb_interface *interface) 
{
	int i;
	
	printk("ni_usb_driver_disconnect\n");
	for(i = 0; i < MAX_NUM_NI_USB_INTERFACES; i++)
	{
		if(ni_usb_driver_interfaces[i] == interface)
		{
			ni_usb_driver_interfaces[i] = NULL;
			break;
		}
	}
	if(i == MAX_NUM_NI_USB_INTERFACES)
	{
		printk("unable to find interface in ni_usb_driver_interfaces[]? bug?\n");
	}
}

// The usbduxsub-driver
static struct usb_driver ni_usb_bus_driver = 
{
	.owner = &__this_module,
	.name = "ni_usb_gpib",
	.probe = ni_usb_driver_probe,
	.disconnect = ni_usb_driver_disconnect,
	.id_table = ni_usb_driver_device_table,
};

static int ni_usb_init_module( void )
{
	int i;
	
	info("ni_usb_gpib driver loading");
	for(i = 0; i < MAX_NUM_NI_USB_INTERFACES; i++)
		ni_usb_driver_interfaces[i] = NULL;
	usb_register(&ni_usb_bus_driver);
	gpib_register_driver(&ni_usb_gpib_interface);

	return 0;
}

static void ni_usb_exit_module( void )
{
	info("ni_usb_gpib driver unloading");
	gpib_unregister_driver(&ni_usb_gpib_interface);
	usb_deregister(&ni_usb_bus_driver);
}

module_init( ni_usb_init_module );
module_exit( ni_usb_exit_module );







