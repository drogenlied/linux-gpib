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

MODULE_LICENSE("GPL");

#define MAX_NUM_NI_USB_INTERFACES = 128
static struct usb_interface *ni_usb_bus_interfaces[MAX_NUM_NI_USB_INTERFACES];

void ni_usb_parse_status_block(struct ni_usb_status_block *status, const uint8_t *buffer)
{
	uint16_t count;
	
	status->id = buffer[0];
	status->ibsta = (buffer[1] << 8) | buffer[2];
	status->error_code = buffer[3];
	count = buffer[4] | (buffer[5] << 8);
	count = ~count;
	count++;
	status->count = count;
};

// interface functions
ssize_t ni_usb_read(gpib_board_t *board, uint8_t *buffer, size_t length, int *end)
{
	int retval;
	ni_usb_private_t *ni_priv = board->private_data;
	struct usb_device *usb_dev;
	int pipe;
	uint8_t *out_data, *in_data;
	int out_data_length, in_data_length;
	int bytes_written, bytes_read;
	int i, j;
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
	out_pipe = usb_sndbulkpipe(usb_dev, 0);
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
	i += ni_usb_bulk_register_write(&out_data[i], NI_USB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(AUXMR), AUX_HLDI);
	i += ni_usb_bulk_register_write(&out_data[i], NI_USB_SUBDEV_TNT4882, nec7210_to_tnt4882_offset(AUXMR), AUX_CLEAR_END);
	while(i % 4)	// pad with zeros to 4-byte boundary
		out_data[i++] = 0x0;
	i += ni_usb_bulk_termination(&out_data[i]);
	retval = usb_bulk_msg(usb_dev, pipe, out_data, i, &bytes_written, HZ /*FIXME set timeout properly */);
	kfree(out_data);
	if(retval || bytes_written != i)
	{
		printk("%s: usb_bulk_msg returned %i, bytes_written=%i, i=%i\n", __FILE__, retval, actual_length, i);		
		return -EIO;
	}
	in_data_length = (length / 15 + 1) * 0x10 + 0x20;
	in_data = kmalloc(in_data_length, GFP_KERNEL);
	if(in_data == NULL) return -ENOMEM;
	in_pipe = usb_rcvbulkpipe(usb_dev, 0);
	retval = usb_bulk_msg(usb_dev, pipe, in_data, in_data_length, &bytes_read, HZ /*FIXME set timeout properly */);
	if(retval)
	{
		printk("%s: usb_bulk_msg returned %i, bytes_read=%i\n", __FILE__, retval, bytes_read);		
		kfree(in_data);
		return -EIO;
	}
	parse_ni_usb_status_block(&status, in_data);
	kfree(in_data);
	//FIXME update ibsta using status info
	return status.count
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
	int i, j;
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
	out_pipe = usb_sndbulkpipe(usb_dev, 0);
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
	in_pipe = usb_rcvbulkpipe(usb_dev, 0);
	retval = usb_bulk_msg(usb_dev, in_pipe, out_data, i, &bytes_written, HZ /*FIXME set timeout properly */);
	kfree(out_data);
	if(retval || bytes_written != i)
	{
		printk("%s: usb_bulk_msg returned %i, bytes_written=%i, i=%i\n", __FILE__, retval, actual_length, i);		
		return -EIO;
	}
	in_data_length = 0x10;
	in_data = kmalloc(in_data_length, GFP_KERNEL);
	if(in_data == NULL) return -ENOMEM;
	retval = usb_bulk_msg(usb_dev, pipe, in_data, in_data_length, &bytes_read, HZ /*FIXME set timeout properly */);
	if(retval || bytes_read != 8)
	{
		printk("%s: usb_bulk_msg returned %i, bytes_read=%i\n", __FILE__, retval, bytes_read);		
		kfree(in_data);
		return -EIO;
	}
	parse_ni_usb_status_block(&status, in_data);
	kfree(in_data);
	//FIXME update ibsta using status info
	return status.count
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
	int i, j;
	int complement_count;
	struct ni_usb_status_block status;
	int timeout_code = 0xd;	//FIXME
	static const int max_command_length = 0xff;
	
	if(length > max_command_length) length = max_command_length;
	usb_dev = interface_to_usbdev(ni_priv->bus_interface);
	out_pipe = usb_sndbulkpipe(usb_dev, 0);
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
	retval = usb_bulk_msg(usb_dev, pipe, out_data, i, &bytes_written, HZ /*FIXME set timeout properly */);
	kfree(out_data);
	if(retval || bytes_written != i)
	{
		printk("%s: usb_bulk_msg returned %i, bytes_written=%i, i=%i\n", __FILE__, retval, actual_length, i);		
		return -EIO;
	}
	in_data_length = 0x10;
	in_data = kmalloc(in_data_length, GFP_KERNEL);
	if(in_data == NULL) return -ENOMEM;
	in_pipe = usb_rcvbulkpipe(usb_dev, 0);
	retval = usb_bulk_msg(usb_dev, in_pipe, in_data, in_data_length, &bytes_read, HZ /*FIXME set timeout properly */);
	if(retval || bytes_read != 8)
	{
		printk("%s: usb_bulk_msg returned %i, bytes_read=%i\n", __FILE__, retval, bytes_read);		
		kfree(in_data);
		return -EIO;
	}
	parse_ni_usb_status_block(&status, in_data);
	kfree(in_data);
	//FIXME update ibsta using status info
	return status.count
}
int ni_usb_take_control(gpib_board_t *board, int synchronous)
{
}
int ni_usb_go_to_standby(gpib_board_t *board)
{
}
void ni_usb_request_system_control( gpib_board_t *board, int request_control )
{
}
void ni_usb_interface_clear(gpib_board_t *board, int assert)
{
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
	static const unsigned int ni_usb_ibsta_mask = SRQI | ATN | CIC;
	
	usb_dev = interface_to_usbdev(ni_priv->bus_interface);
	pipe = usb_rcvctrlpipe(usb_dev, 0);
	buffer = kmalloc(bufferLength, GFP_KERNEL);
	if(buffer == NULL)
	{
		return -ENOMEM;
	}
	retval = usb_control_msg(usb_dev, pipe, USB_DIR_IN | USB_TYPE_VENDOR, 
		0x21, 0x200, 0x0, buffer, bufferLength, HZ);
	if(retval != sizeof(buffer)
	{
		printk("%s: usb_control_msg returned %i\n", __FILE__, retval);		
		kfree(buffer);
		return -1;
	}
	parse_ni_usb_status_block(&status, buffer);
	kfree(buffer);
	board->status &= ~clear_mask;
	board->status &= ni_usb_ibsta_mask;
	board->status |= status->ibsta & ni_usb_ibsta_mask;	
	if(status->ibsta & ~ni_usb_ibsta_mask)
	{
		printk("%s: debug: ibsta from status block is 0x%x\n", status->ibsta);
	}
}
void ni_usb_primary_address(gpib_board_t *board, unsigned int address)
{
}
void ni_usb_secondary_address(gpib_board_t *board, unsigned int address, int enable)
{
}
int ni_usb_parallel_poll(gpib_board_t *board, uint8_t *result)
{
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

int ni_usb_attach(gpib_board_t *board)
{
	int retval;
	
	retval = ni_usb_allocate_private(board);
	if(retval < 0) return retval;
	/*FIXME: should allow user to specifiy which device he wants to attach.
	 Use usb_make_path() */
	for(i = 0; i < MAX_NUM_NI_USB_INTERFACES; i++)
	{
		if(ni_usb_bus_interfaces[i])
		{
			board->bus_interface = ni_usb_bus_interfaces[i];
			break;
		}
	}
	if(i == MAX_NUM_NI_USB_INTERFACES)
	{
		printk("no NI usb-b gpib adapters found\n");
		return -1;
	}
	
	return 0;
}

void ni_usb_detach(gpib_board_t *board)
{
}

gpib_interface_t ni_usb_gpib_interface =
{
	name: "ni_usb_b",
	attach: ni_usb_attach,
	detach: ni_usb_detach,
	read: ni_usb_accel_read,
	write: ni_usb_accel_write,
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


MODULE_DEVICE_TABLE(usb, ni_usb_device_table);

static int ni_usb_driver_probe(struct usb_interface *interface, 
	const struct usb_device_id *id) 
{
	printk("ni_usb_driver_probe\n");
	for(i = 0; i < MAX_NUM_NI_USB_INTERFACES; i++)
	{
		if(ni_usb_bus_interfaces[i] == NULL)
		{
			ni_usb_bus_interfaces[i] = interface;
			break;
		}
	}
	if(i == MAX_NUM_NI_USB_INTERFACES)
	{
		printk("out of space in ni_usb_bus_interfaces[]\n");
		return -1;
	}
	return 0;
}

static void ni_usb_driver_disconnect(struct usb_interface *interface) 
{
	printk("ni_usb_driver_disconnect\n");
	for(i = 0; i < MAX_NUM_NI_USB_INTERFACES; i++)
	{
		if(ni_usb_bus_interfaces[i] == interface)
		{
			ni_usb_bus_interfaces[i] = NULL;
			break;
		}
	}
	if(i == MAX_NUM_NI_USB_INTERFACES)
	{
		printk("unable to find interface in ni_usb_bus_interfaces[]? bug?\n");
	}
}

// The usbduxsub-driver
static struct usb_driver ni_usb_bus_driver = 
{
	.owner = &__this_module,
	.name = "ni_usb_gpib",
	.probe = ni_usb_driver_probe,
	.disconnect = ni_usb_driver_disconnect,
	.id_table = ni_usb_driver_devicetable,
};

static int ni_usb_init_module( void )
{
	info("ni_usb_gpib driver loading");
	for(i = 0; i < MAX_NUM_NI_USB_INTERFACES; i++)
		ni_usb_bus_interfaces[i] = NULL;
	usb_register(&ni_usb_driver);
	gpib_register_driver(&ni_usb_gpib_interface);

	return 0;
}

static void ni_usb_exit_module( void )
{
	info("ni_usb_gpib driver unloading");
	gpib_unregister_driver(&ni_usb_gpib_interface);
	usb_deregister(&ni_usb_driver);
}

module_init( ni_usb_init_module );
module_exit( ni_usb_exit_module );







