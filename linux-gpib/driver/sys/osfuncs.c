/***************************************************************************
                               sys/osfuncs.c
                             -------------------

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

#include <ibsys.h>

#include <linux/fcntl.h>

int board_type_ioctl(gpib_board_t *board, unsigned long arg);
int read_ioctl(gpib_board_t *board, unsigned long arg);
int write_ioctl(gpib_board_t *board, unsigned long arg);
int command_ioctl(gpib_board_t *board, unsigned long arg);
int status_ioctl(gpib_board_t *board, unsigned long arg);
int open_dev_ioctl( struct file *filep, gpib_board_t *board, unsigned long arg );
int close_dev_ioctl( struct file *filep, gpib_board_t *board, unsigned long arg );
int cleanup_open_devices( struct file *filep, gpib_board_t *board );
int serial_poll_ioctl( gpib_board_t *board, unsigned long arg );
int wait_ioctl( gpib_board_t *board, unsigned long arg );
int parallel_poll_ioctl( gpib_board_t *board, unsigned long arg );
int auto_poll_enable_ioctl( gpib_board_t *board, unsigned long arg );
int online_ioctl( gpib_board_t *board, unsigned long arg );
int remote_enable_ioctl( gpib_board_t *board, unsigned long arg );
int take_control_ioctl( gpib_board_t *board, unsigned long arg );
int line_status_ioctl( gpib_board_t *board, unsigned long arg );
int pad_ioctl( gpib_board_t *board, unsigned long arg );
int sad_ioctl( gpib_board_t *board, unsigned long arg );
int eos_ioctl( gpib_board_t *board, unsigned long arg );
int request_service_ioctl( gpib_board_t *board, unsigned long arg );
int iobase_ioctl( gpib_board_t *board, unsigned long arg );
int irq_ioctl( gpib_board_t *board, unsigned long arg );
int dma_ioctl( gpib_board_t *board, unsigned long arg );

int ibopen(struct inode *inode, struct file *filep)
{
	unsigned int minor = MINOR(inode->i_rdev);
	gpib_board_t *board;

	if(minor >= MAX_NUM_GPIB_BOARDS)
	{
		printk("gpib: invalid minor number of device file\n");
		return -ENODEV;
	}

	board = &board_array[minor];

	filep->private_data = kmalloc( sizeof( struct list_head ), GFP_KERNEL );
	if( filep->private_data == NULL )
	{
		return -ENOMEM;
	}
	INIT_LIST_HEAD( ( struct list_head * ) filep->private_data );

	if( board->exclusive )
	{
		return -EBUSY;
	}

	if ( filep->f_flags & O_EXCL )
	{
		if ( board->open_count )
		{
			return -EBUSY;
		}
		board->exclusive = 1;
	}

	board->open_count++;

	return 0;
}


int ibclose(struct inode *inode, struct file *filep)
{
	unsigned int minor = MINOR(inode->i_rdev);
	gpib_board_t *board;

	if(minor >= MAX_NUM_GPIB_BOARDS)
	{
		printk("gpib: invalid minor number of device file\n");
		return -ENODEV;
	}

	board = &board_array[ minor ];

	if( board->online && board->open_count == 1 )
		iboffline( board );

	board->open_count--;

	if( board->exclusive )
		board->exclusive = 0;

	if( filep->private_data )
	{
		cleanup_open_devices( filep, board );
		kfree( filep->private_data );
	}

	return 0;
}

int ibioctl(struct inode *inode, struct file *filep, unsigned int cmd, unsigned long arg)
{
	int	retval = 0; 		/* assume everything OK for now */
	unsigned int minor = MINOR(inode->i_rdev);
	gpib_board_t *board;

	if(minor >= MAX_NUM_GPIB_BOARDS)
	{
		printk("gpib: invalid minor number of device file\n");
		return -ENODEV;
	}
	board = &board_array[minor];

	/* XXX lock other processes from performing commands */
	retval = down_interruptible(&board->mutex);
	if(retval)
	{
		printk("gpib: ioctl interrupted while waiting on lock\n");
		return -ERESTARTSYS;
	}

	GPIB_DPRINTK( "minor %i ioctl %i\n", minor, cmd);

	if( board->interface == NULL && cmd != CFCBOARDTYPE )
	{
		printk("gpib: no gpib board configured on /dev/gpib%i\n", minor);
		up( &board->mutex );
		return -ENODEV;
	}

	switch( cmd )
	{
		case CFCBOARDTYPE:
			retval = board_type_ioctl(board, arg);
			break;
		case IBRD:
			retval = read_ioctl( board, arg );
			break;
		case IBWRT:
			retval = write_ioctl( board, arg );
			break;
		case IBCMD:
			retval = command_ioctl( board, arg );
			break;
		case IBSTATUS:
			retval = status_ioctl( board, arg );
			break;
		case IBTMO:
			retval = ibtmo( board, arg );
			break;
		case IBOPENDEV:
			retval = open_dev_ioctl( filep, board, arg );
			break;
		case IBCLOSEDEV:
			retval = close_dev_ioctl( filep, board, arg );
			break;
		case IBRSP:
			retval = serial_poll_ioctl( board, arg );
			break;
		case IBWAIT:
			retval = wait_ioctl( board, arg );
			break;
		case IBRPP:
			retval = parallel_poll_ioctl( board, arg );
			break;
		case IBAPE:
			retval = auto_poll_enable_ioctl( board, arg );
			break;
		case IBONL:
			retval = online_ioctl( board, arg );
			break;
		case IBSIC:
			retval = ibsic( board );
			break;
		case IBSRE:
			retval = remote_enable_ioctl( board, arg );
			break;
		case IBGTS:
			retval = ibgts( board );
			break;
		case IBCAC:
			retval = take_control_ioctl( board, arg );
			break;
		case IBLINES:
			retval = line_status_ioctl( board, arg );
			break;
		case IBPAD:
			retval = pad_ioctl( board, arg );
			break;
		case IBSAD:
			retval = sad_ioctl( board, arg );
			break;
		case IBEOS:
			retval = eos_ioctl( board, arg );
			break;
		case IBRSV:
			retval = request_service_ioctl( board, arg );
			break;
		case CFCBASE:
			retval = iobase_ioctl( board, arg );
			break;
		case CFCIRQ:
			retval = irq_ioctl( board, arg );
			break;
		case CFCDMA:
			retval = dma_ioctl( board, arg );
			break;
		default:
			retval = -ENOTTY;
			break;
	}

	up( &board->mutex );
	return retval;
}

int board_type_ioctl(gpib_board_t *board, unsigned long arg)
{
	struct list_head *list_ptr;
	board_type_ioctl_t cmd;
	int retval;

	retval = copy_from_user(&cmd, (void*)arg, sizeof(board_type_ioctl_t));
	if(retval)
	{
		return retval;
	}

	for(list_ptr = registered_drivers.next; list_ptr != &registered_drivers; list_ptr = list_ptr->next)
	{
		gpib_interface_t *interface;

		interface = list_entry(list_ptr, gpib_interface_t, list);
		if(strcmp(interface->name, cmd.name) == 0)
		{
			board->interface = interface;
			return 0;
		}
	}

	return -EINVAL;
}

int read_ioctl(gpib_board_t *board, unsigned long arg)
{
	read_write_ioctl_t read_cmd;
	uint8_t *userbuf;
	unsigned long remain;
	int end_flag = 0;
	int retval;
	ssize_t ret;

	retval = copy_from_user(&read_cmd, (void*) arg, sizeof(read_cmd));
	if (retval)
		return -EFAULT;

	/* Check write access to buffer */
	retval = verify_area(VERIFY_WRITE, read_cmd.buffer, read_cmd.count);
	if (retval)
		return -EFAULT;

	/* Read buffer loads till we fill the user supplied buffer */
	userbuf = read_cmd.buffer;
	remain = read_cmd.count;
	while(remain > 0 && end_flag == 0)
	{
		ret = ibrd(board, board->buffer, (board->buffer_length < remain) ? board->buffer_length :
			remain, &end_flag);
		if(ret < 0)
		{
			return -EIO;
		}
		copy_to_user(userbuf, board->buffer, ret);
		remain -= ret;
		userbuf += ret;
	}
	read_cmd.count -= remain;
	read_cmd.end = end_flag;

	retval = copy_to_user((void*) arg, &read_cmd, sizeof(read_cmd));
	if(retval) return -EFAULT;

	return 0;
}

int command_ioctl(gpib_board_t *board, unsigned long arg)
{
	read_write_ioctl_t cmd;
	uint8_t *userbuf;
	unsigned long remain;
	int retval;
	ssize_t ret;

	retval = copy_from_user(&cmd, (void*) arg, sizeof(cmd));
	if (retval)
		return -EFAULT;

	/* Check read access to buffer */
	retval = verify_area(VERIFY_READ, cmd.buffer, cmd.count);
	if (retval)
		return -EFAULT;

	/* Write buffer loads till we empty the user supplied buffer */
	userbuf = cmd.buffer;
	remain = cmd.count;
	while (remain > 0 && !(ibstatus(board) & (TIMO)))
	{
		copy_from_user(board->buffer, userbuf, (board->buffer_length < remain) ?
			board->buffer_length : remain );
		ret = ibcmd(board, board->buffer, (board->buffer_length < remain) ?
			board->buffer_length : remain );
		if(ret < 0)
		{
			retval = -EIO;
			break;
		}
		remain -= ret;
		userbuf += ret;
	}

	cmd.count -= remain;

	retval = copy_to_user((void*) arg, &cmd, sizeof(cmd));
	if(retval) return -EFAULT;

	return 0;
}

int write_ioctl(gpib_board_t *board, unsigned long arg)
{
	read_write_ioctl_t write_cmd;
	uint8_t *userbuf;
	unsigned long remain;
	int retval;
	ssize_t ret;

	retval = copy_from_user(&write_cmd, (void*) arg, sizeof(write_cmd));
	if (retval)
		return -EFAULT;

	/* Check read access to buffer */
	retval = verify_area(VERIFY_READ, write_cmd.buffer, write_cmd.count);
	if (retval)
		return -EFAULT;

	/* Write buffer loads till we empty the user supplied buffer */
	userbuf = write_cmd.buffer;
	remain = write_cmd.count;
	while(remain > 0)
	{
		int send_eoi;
		send_eoi = remain <= board->buffer_length && write_cmd.end;
		copy_from_user(board->buffer, userbuf, (board->buffer_length < remain) ?
			board->buffer_length : remain );
		ret = ibwrt(board, board->buffer, (board->buffer_length < remain) ?
			board->buffer_length : remain, send_eoi);
		if(ret < 0)
		{
			retval = -EIO;
			break;
		}
		remain -= ret;
		userbuf += ret;
	}

	write_cmd.count -= remain;

	retval = copy_to_user((void*) arg, &write_cmd, sizeof(write_cmd));
	if(retval) return -EFAULT;

	return 0;
}

int status_ioctl(gpib_board_t *board, unsigned long arg)
{
	int status;
	int retval;

	status = ibstatus(board);

	retval = put_user( status, (int *) arg );
	if (retval)
		return -EFAULT;

	return 0;
}

int increment_open_device_count( struct list_head *head, unsigned int pad, int sad )
{
	struct list_head *list_ptr;
	gpib_device_t *device;

	/* first see if address has already been opened, then increment
	 * open count */
	for( list_ptr = head->next; list_ptr != head; list_ptr = list_ptr->next )
	{
		device = list_entry( list_ptr, gpib_device_t, list );
		if( device->pad == pad &&
			( device->sad == sad ) )
		{
			GPIB_DPRINTK( "incrementing open count for pad %i, sad %i\n",
				device->pad, device->sad );
			device->reference_count++;
			return 0;
		}
	}

	/* otherwise we need to allocate a new gpib_device_t */
	device = kmalloc( sizeof( gpib_device_t ), GFP_KERNEL );
	if( device == NULL )
		return -ENOMEM;
	init_gpib_device( device );
	device->pad = pad;
	device->sad = sad;
	device->reference_count = 1;

	list_add( &device->list, head );

	GPIB_DPRINTK( "opened pad %i, sad %i\n",
		device->pad, device->sad );

	return 0;
}

int subtract_open_device_count( struct list_head *head, unsigned int pad, int sad, unsigned int count )
{
	gpib_device_t *device;
	struct list_head *list_ptr;

	for( list_ptr = head->next; list_ptr != head; list_ptr = list_ptr->next )
	{
		device = list_entry( list_ptr, gpib_device_t, list );
		if( device->pad == pad &&
			device->sad == sad )
		{
			GPIB_DPRINTK( "decrementing open count for pad %i, sad %i\n",
				device->pad, device->sad );
			if( count > device->reference_count )
			{
				printk( "gpib: bug! in subtract_open_device_count()\n" );
				return -EINVAL;
			}
			device->reference_count -= count;
			if( device->reference_count == 0 )
			{
				GPIB_DPRINTK( "closing pad %i, sad %i\n",
					device->pad, device->sad );
				list_del( list_ptr );
				kfree( device );
			}
			return 0;
		}
	}
	printk( "gpib: bug! tried to close address that was never opened!\n" );
	return -EINVAL;
}

inline int decrement_open_device_count( struct list_head *head, unsigned int pad, int sad )
{
	return subtract_open_device_count( head, pad, sad, 1 );
}

int cleanup_open_devices( struct file *filep, gpib_board_t *board )
{
	struct list_head *list_ptr, *head = filep->private_data;
	gpib_device_t *device;
	int retval = 0;

	list_ptr = head->next;
	while( list_ptr != head )
	{
		device = list_entry( list_ptr, gpib_device_t, list );
		retval = subtract_open_device_count( &board->device_list, device->pad, device->sad,
			device->reference_count );
		if( retval < 0 ) break;
		list_del( list_ptr );
		list_ptr = list_ptr->next;
		kfree( device );
	}

	return retval;
}

int open_dev_ioctl( struct file *filep, gpib_board_t *board, unsigned long arg )
{
	open_close_dev_ioctl_t open_dev_cmd;
	int retval;
	struct list_head *list_ptr = filep->private_data;

	retval = copy_from_user( &open_dev_cmd, ( void* ) arg, sizeof( open_dev_cmd ) );
	if (retval)
		return -EFAULT;

	retval = increment_open_device_count( list_ptr, open_dev_cmd.pad, open_dev_cmd.sad );
	if( retval < 0 )
		return retval;
	retval = increment_open_device_count( &board->device_list, open_dev_cmd.pad, open_dev_cmd.sad );
	if( retval < 0 )
	{
		decrement_open_device_count( list_ptr, open_dev_cmd.pad, open_dev_cmd.sad );
		return retval;
	}

	return 0;
}

int close_dev_ioctl( struct file *filep, gpib_board_t *board, unsigned long arg )
{
	open_close_dev_ioctl_t close_dev_cmd;
	struct list_head *list_ptr = filep->private_data;
	int retval;

	retval = copy_from_user( &close_dev_cmd, ( void* ) arg, sizeof( close_dev_cmd ) );
	if (retval)
		return -EFAULT;

	retval = decrement_open_device_count( list_ptr, close_dev_cmd.pad, close_dev_cmd.sad );
	if( retval < 0 ) return retval;
	retval = decrement_open_device_count( &board->device_list, close_dev_cmd.pad, close_dev_cmd.sad );
	if( retval < 0 ) return retval;

	return 0;
}

gpib_device_t * get_gpib_device( gpib_board_t *board, unsigned int pad, int sad )
{
	gpib_device_t *device;
	struct list_head *list_ptr;
	const struct list_head *head = &board->device_list;

	for( list_ptr = head->next; list_ptr != head; list_ptr = list_ptr->next )
	{
		device = list_entry( list_ptr, gpib_device_t, list );
		if( device->pad == pad && device->sad == sad )
			return device;
	}

	return NULL;
}

#if 0
unsigned int num_status_bytes( const gpib_device_t *dev )
{
	struct list_head *list_ptr;
	const struct list_head *head = &device->serial_poll_bytes;
	unsigned int count;

	count = 0;
	for( list_ptr = head->next; list_ptr != head; list_ptr = list_ptr->next )
		count++

	GPIB_DPRINTK( "%i status bytes stored for pad %i, sad %i\n", count,
		dev->pad, dev->sad );

	return count;
}
#endif

int serial_poll_ioctl( gpib_board_t *board, unsigned long arg )
{
	serial_poll_ioctl_t serial_cmd;
	int retval;

	retval = copy_from_user( &serial_cmd, ( void* ) arg, sizeof( serial_cmd ) );
	if( retval )
		return -EFAULT;
//XXX check for autopoll
	retval = dvrsp( board, serial_cmd.pad, serial_cmd.sad, board->usec_timeout,
		&serial_cmd.status_byte );
	if( retval < 0 )
		return retval;

	retval = copy_to_user( ( void * ) arg, &serial_cmd, sizeof( serial_cmd ) );
	if( retval )
		return -EFAULT;

	return 0;
}

int wait_ioctl( gpib_board_t *board, unsigned long arg )
{
	unsigned int wait_mask;
	int retval;

	retval = copy_from_user( &wait_mask, ( void * ) arg, sizeof( wait_mask ) );
	if( retval )
		return -EFAULT;

	return ibwait( board, wait_mask );
}

int parallel_poll_ioctl( gpib_board_t *board, unsigned long arg )
{
	uint8_t poll_byte;
	int retval;

	retval = ibrpp( board, &poll_byte );
	if( retval < 0 )
		return retval;

	retval = copy_to_user( ( void * ) arg, &poll_byte, sizeof( poll_byte ) );
	if( retval )
		return -EFAULT;

	return 0;
}

int auto_poll_enable_ioctl( gpib_board_t *board, unsigned long arg )
{
	int enable;
	int retval;

	retval = copy_from_user( &enable, ( void * ) arg, sizeof( enable ) );
	if( retval )
		return -EFAULT;

	if( enable )
		board->auto_poll = 1;
	else
		board->auto_poll = 0;

	return 0;
}

int online_ioctl( gpib_board_t *board, unsigned long arg )
{
	int online;
	int retval;

	retval = copy_from_user( &online, ( void * ) arg, sizeof( online ) );
	if( retval )
		return -EFAULT;

	if( online )
		return ibonline( board );
	else
		return iboffline( board );

	return 0;
}

int remote_enable_ioctl( gpib_board_t *board, unsigned long arg )
{
	int enable;
	int retval;

	retval = copy_from_user( &enable, ( void * ) arg, sizeof( enable ) );
	if( retval )
		return -EFAULT;

	return ibsre( board, enable );
}

int take_control_ioctl( gpib_board_t *board, unsigned long arg )
{
	int synchronous;
	int retval;

	retval = copy_from_user( &synchronous, ( void * ) arg, sizeof( synchronous ) );
	if( retval )
		return -EFAULT;

	return ibcac( board, synchronous );
}

int line_status_ioctl( gpib_board_t *board, unsigned long arg )
{
	short lines;
	int retval;

	retval = iblines( board, &lines );
	if( retval < 0 )
		return retval;

	retval = copy_to_user( ( void * ) arg, &lines, sizeof( lines ) );
	if( retval )
		return -EFAULT;

	return 0;
}

int pad_ioctl( gpib_board_t *board, unsigned long arg )
{
	unsigned int address;
	int retval;

	retval = copy_from_user( &address, ( void * ) arg, sizeof( address ) );
	if( retval )
		return -EFAULT;

	return ibpad( board, address );
}

int sad_ioctl( gpib_board_t *board, unsigned long arg )
{
	int address;
	int retval;

	retval = copy_from_user( &address, ( void * ) arg, sizeof( address ) );
	if( retval )
		return -EFAULT;

	return ibsad( board, address );
}

int eos_ioctl( gpib_board_t *board, unsigned long arg )
{
	eos_ioctl_t eos_cmd;
	int retval;

	retval = copy_from_user( &eos_cmd, ( void * ) arg, sizeof( eos_cmd ) );
	if( retval )
		return -EFAULT;

	return ibeos( board, eos_cmd.eos, eos_cmd.eos_flags );
}

int request_service_ioctl( gpib_board_t *board, unsigned long arg )
{
	uint8_t status_byte;
	int retval;

	retval = copy_from_user( &status_byte, ( void * ) arg, sizeof( status_byte ) );
	if( retval )
		return -EFAULT;

	return ibrsv( board, status_byte );
}

int iobase_ioctl( gpib_board_t *board, unsigned long arg )
{
	unsigned long base_addr;
	int retval;

	retval = copy_from_user( &base_addr, ( void * ) arg, sizeof( base_addr ) );
	if( retval )
		return -EFAULT;

	board->ibbase = base_addr;

	return 0;
}

int irq_ioctl( gpib_board_t *board, unsigned long arg )
{
	unsigned int irq;
	int retval;

	retval = copy_from_user( &irq, ( void * ) arg, sizeof( irq ) );
	if( retval )
		return -EFAULT;

	board->ibirq = irq;

	return 0;
}

int dma_ioctl( gpib_board_t *board, unsigned long arg )
{
	unsigned int dma_channel;
	int retval;

	retval = copy_from_user( &dma_channel, ( void * ) arg, sizeof( dma_channel ) );
	if( retval )
		return -EFAULT;

	board->ibdma = dma_channel;

	return 0;
}