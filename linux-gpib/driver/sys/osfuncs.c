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

#include "ibsys.h"
#include "autopoll.h"

#include <linux/fcntl.h>

static int board_type_ioctl(gpib_board_t *board, unsigned long arg);
static int read_ioctl(gpib_board_t *board, unsigned long arg);
static int write_ioctl(gpib_board_t *board, unsigned long arg);
static int command_ioctl(gpib_board_t *board, unsigned long arg);
static int status_ioctl(gpib_board_t *board, unsigned long arg);
static int open_dev_ioctl( struct file *filep, gpib_board_t *board, unsigned long arg );
static int close_dev_ioctl( struct file *filep, gpib_board_t *board, unsigned long arg );
static int serial_poll_ioctl( gpib_board_t *board, unsigned long arg );
static int wait_ioctl( gpib_board_t *board, unsigned long arg );
static int parallel_poll_ioctl( gpib_board_t *board, unsigned long arg );
static int auto_poll_enable_ioctl( gpib_board_t *board, unsigned long arg );
static int online_ioctl( gpib_board_t *board, unsigned long arg );
static int remote_enable_ioctl( gpib_board_t *board, unsigned long arg );
static int take_control_ioctl( gpib_board_t *board, unsigned long arg );
static int line_status_ioctl( gpib_board_t *board, unsigned long arg );
static int pad_ioctl( gpib_board_t *board, unsigned long arg );
static int sad_ioctl( gpib_board_t *board, unsigned long arg );
static int eos_ioctl( gpib_board_t *board, unsigned long arg );
static int request_service_ioctl( gpib_board_t *board, unsigned long arg );
static int iobase_ioctl( gpib_board_t *board, unsigned long arg );
static int irq_ioctl( gpib_board_t *board, unsigned long arg );
static int dma_ioctl( gpib_board_t *board, unsigned long arg );
static int autopoll_ioctl( gpib_board_t *board);
static int mutex_ioctl( gpib_board_t *board, unsigned long arg );

static int cleanup_open_devices( gpib_file_private_t *file_priv, gpib_board_t *board );

static void init_gpib_file_private( gpib_file_private_t *priv )
{
	INIT_LIST_HEAD( &priv->device_list );
	priv->holding_mutex = 0;
}

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

	filep->private_data = kmalloc( sizeof( gpib_file_private_t ), GFP_KERNEL );
	if( filep->private_data == NULL )
	{
		return -ENOMEM;
	}
	init_gpib_file_private( ( gpib_file_private_t * ) filep->private_data );

	GPIB_DPRINTK( "gpib: opening minor %d\n", minor );

	board->open_count++;

	return 0;
}


int ibclose(struct inode *inode, struct file *filep)
{
	unsigned int minor = MINOR(inode->i_rdev);
	gpib_board_t *board;
	gpib_file_private_t *priv = filep->private_data;

	if(minor >= MAX_NUM_GPIB_BOARDS)
	{
		printk("gpib: invalid minor number of device file\n");
		return -ENODEV;
	}

	GPIB_DPRINTK( "gpib: closing minor %d\n", minor );

	board = &board_array[ minor ];

	if( priv )
	{
		cleanup_open_devices( priv, board );
		if( priv->holding_mutex )
			up( &board->mutex );
		kfree( filep->private_data );
	}

	if( board->online && board->open_count == 1 )
		iboffline( board );

	board->open_count--;

	if( board->exclusive )
		board->exclusive = 0;

	return 0;
}



int ibioctl(struct inode *inode, struct file *filep, unsigned int cmd, unsigned long arg)
{
	unsigned int minor = MINOR(inode->i_rdev);
	gpib_board_t *board;

	if( minor >= MAX_NUM_GPIB_BOARDS )
	{
		printk("gpib: invalid minor number of device file\n");
		return -ENODEV;
	}
	board = &board_array[ minor ];

	GPIB_DPRINTK( "minor %i ioctl %d, ifc=%s, open=%d, onl=%d\n",
		      minor, cmd&0xff,
		      board->interface ? board->interface->name : "",
		      board->open_count,
		      board->online );

	if( board->interface == NULL && cmd != CFCBOARDTYPE )
	{
		printk("gpib: no gpib board configured on /dev/gpib%i\n", minor);
		up( &board->mutex );
		return -ENODEV;
	}

	switch( cmd )
	{
		case CFCBOARDTYPE:
			return board_type_ioctl(board, arg);
			break;
		case CFCBASE:
			if (board->online) return -EINVAL;
			return iobase_ioctl( board, arg );
			break;
		case CFCIRQ:
			if (board->online) return -EINVAL;
			return irq_ioctl( board, arg );
			break;
		case CFCDMA:
			if (board->online) return -EINVAL;
			return dma_ioctl( board, arg );
			break;
		case IBMUTEX:
			return mutex_ioctl( board, arg );
			break;
		case IBONL:
			return online_ioctl( board, arg );
			break;
		default:
			break;
	}

	if ( !board->online )
	{
		printk( "gpib: invalid ioctl for offline board\n" );
		return -EINVAL;
	}

	switch( cmd )
	{
		case IBRD:
			return read_ioctl( board, arg );
			break;
		case IBWRT:
			return write_ioctl( board, arg );
			break;
		case IBCMD:
			return command_ioctl( board, arg );
			break;
		case IBSTATUS:
			return status_ioctl( board, arg );
			break;
		case IBTMO:
			return ibtmo( board, arg );
			break;
		case IBOPENDEV:
			return open_dev_ioctl( filep, board, arg );
			break;
		case IBCLOSEDEV:
			return close_dev_ioctl( filep, board, arg );
			break;
		case IBRSP:
			return serial_poll_ioctl( board, arg );
			break;
		case IBWAIT:
			return wait_ioctl( board, arg );
			break;
		case IBRPP:
			return parallel_poll_ioctl( board, arg );
			break;
		case IBAPE:
			return auto_poll_enable_ioctl( board, arg );
			break;
		case IBSIC:
			return ibsic( board );
			break;
		case IBSRE:
			return remote_enable_ioctl( board, arg );
			break;
		case IBGTS:
			return ibgts( board );
			break;
		case IBCAC:
			return take_control_ioctl( board, arg );
			break;
		case IBLINES:
			return line_status_ioctl( board, arg );
			break;
		case IBPAD:
			return pad_ioctl( board, arg );
			break;
		case IBSAD:
			return sad_ioctl( board, arg );
			break;
		case IBEOS:
			return eos_ioctl( board, arg );
			break;
		case IBRSV:
			return request_service_ioctl( board, arg );
			break;
		case IBAUTOPOLL:
			return autopoll_ioctl( board );
			break;
		default:
			return -ENOTTY;
			break;
	}

	return -ENOTTY;
}

static int board_type_ioctl(gpib_board_t *board, unsigned long arg)
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

static int read_ioctl(gpib_board_t *board, unsigned long arg)
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

static int command_ioctl(gpib_board_t *board, unsigned long arg)
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

static int write_ioctl(gpib_board_t *board, unsigned long arg)
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

static int status_ioctl(gpib_board_t *board, unsigned long arg)
{
	int status;
	int retval;

	status = ibstatus(board);

	retval = put_user( status, (int *) arg );
	if (retval)
		return -EFAULT;

	return 0;
}

static int increment_open_device_count( struct list_head *head, unsigned int pad, int sad )
{
	struct list_head *list_ptr;
	gpib_device_t *device;

	/* first see if address has already been opened, then increment
	 * open count */
	for( list_ptr = head->next; list_ptr != head; list_ptr = list_ptr->next )
	{
		device = list_entry( list_ptr, gpib_device_t, list );
		if( gpib_address_equal( device->pad, device->sad, pad, sad ) )
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

static int subtract_open_device_count( struct list_head *head, unsigned int pad, int sad, unsigned int count )
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

static inline int decrement_open_device_count( struct list_head *head, unsigned int pad, int sad )
{
	return subtract_open_device_count( head, pad, sad, 1 );
}

static int cleanup_open_devices( gpib_file_private_t *file_priv, gpib_board_t *board )
{
	struct list_head *list_ptr, *head = &file_priv->device_list;
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

static int open_dev_ioctl( struct file *filep, gpib_board_t *board, unsigned long arg )
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

static int close_dev_ioctl( struct file *filep, gpib_board_t *board, unsigned long arg )
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

static int serial_poll_ioctl( gpib_board_t *board, unsigned long arg )
{
	serial_poll_ioctl_t serial_cmd;
	int retval;

	retval = copy_from_user( &serial_cmd, ( void* ) arg, sizeof( serial_cmd ) );
	if( retval )
		return -EFAULT;

	retval = get_serial_poll_byte( board, serial_cmd.pad, serial_cmd.sad, board->usec_timeout,
		&serial_cmd.status_byte );
	if( retval < 0 )
		return retval;

	retval = copy_to_user( ( void * ) arg, &serial_cmd, sizeof( serial_cmd ) );
	if( retval )
		return -EFAULT;

	return 0;
}

static int wait_ioctl( gpib_board_t *board, unsigned long arg )
{
	wait_ioctl_t wait_cmd;
	int retval;

	retval = copy_from_user( &wait_cmd, ( void * ) arg, sizeof( wait_cmd ) );
	if( retval )
		return -EFAULT;

	return ibwait( board, wait_cmd.mask, wait_cmd.pad, wait_cmd.sad );
}

static int parallel_poll_ioctl( gpib_board_t *board, unsigned long arg )
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

static int auto_poll_enable_ioctl( gpib_board_t *board, unsigned long arg )
{
	int enable;
	int retval;

	retval = copy_from_user( &enable, ( void * ) arg, sizeof( enable ) );
	if( retval )
		return -EFAULT;

	if( enable )
		board->autopoll = 1;
	else
		board->autopoll = 0;

	return 0;
}

static int online_ioctl( gpib_board_t *board, unsigned long arg )
{
	online_ioctl_t online_cmd;
	int retval;

	retval = copy_from_user( &online_cmd, ( void * ) arg, sizeof( online_cmd ) );
	if( retval )
		return -EFAULT;

	if( online_cmd.online )
		return ibonline( board, online_cmd.master );
	else
		return iboffline( board );

	return 0;
}

static int remote_enable_ioctl( gpib_board_t *board, unsigned long arg )
{
	int enable;
	int retval;

	retval = copy_from_user( &enable, ( void * ) arg, sizeof( enable ) );
	if( retval )
		return -EFAULT;

	return ibsre( board, enable );
}

static int take_control_ioctl( gpib_board_t *board, unsigned long arg )
{
	int synchronous;
	int retval;

	retval = copy_from_user( &synchronous, ( void * ) arg, sizeof( synchronous ) );
	if( retval )
		return -EFAULT;

	return ibcac( board, synchronous );
}

static int line_status_ioctl( gpib_board_t *board, unsigned long arg )
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

static int pad_ioctl( gpib_board_t *board, unsigned long arg )
{
	unsigned int address;
	int retval;

	retval = copy_from_user( &address, ( void * ) arg, sizeof( address ) );
	if( retval )
		return -EFAULT;

	return ibpad( board, address );
}

static int sad_ioctl( gpib_board_t *board, unsigned long arg )
{
	int address;
	int retval;

	retval = copy_from_user( &address, ( void * ) arg, sizeof( address ) );
	if( retval )
		return -EFAULT;

	return ibsad( board, address );
}

static int eos_ioctl( gpib_board_t *board, unsigned long arg )
{
	eos_ioctl_t eos_cmd;
	int retval;

	retval = copy_from_user( &eos_cmd, ( void * ) arg, sizeof( eos_cmd ) );
	if( retval )
		return -EFAULT;

	return ibeos( board, eos_cmd.eos, eos_cmd.eos_flags );
}

static int request_service_ioctl( gpib_board_t *board, unsigned long arg )
{
	uint8_t status_byte;
	int retval;

	retval = copy_from_user( &status_byte, ( void * ) arg, sizeof( status_byte ) );
	if( retval )
		return -EFAULT;

	return ibrsv( board, status_byte );
}

static int iobase_ioctl( gpib_board_t *board, unsigned long arg )
{
	unsigned long base_addr;
	int retval;

	retval = copy_from_user( &base_addr, ( void * ) arg, sizeof( base_addr ) );
	if( retval )
		return -EFAULT;

	board->ibbase = base_addr;

	return 0;
}

static int irq_ioctl( gpib_board_t *board, unsigned long arg )
{
	unsigned int irq;
	int retval;

	retval = copy_from_user( &irq, ( void * ) arg, sizeof( irq ) );
	if( retval )
		return -EFAULT;

	board->ibirq = irq;

	return 0;
}

static int dma_ioctl( gpib_board_t *board, unsigned long arg )
{
	unsigned int dma_channel;
	int retval;

	retval = copy_from_user( &dma_channel, ( void * ) arg, sizeof( dma_channel ) );
	if( retval )
		return -EFAULT;

	board->ibdma = dma_channel;

	return 0;
}

static int autopoll_ioctl( gpib_board_t *board )
{
	int retval = 0;

	if( down_interruptible( &board->autopoll_mutex ) )
	{
		return -ERESTARTSYS;
	}

	while( 1 )
	{
		if( wait_event_interruptible( board->wait,
			board->online &&
			board->autopoll &&
			board->stuck_srq == 0 &&
			test_and_clear_bit( SRQI_NUM, &board->status) ) )
		{
			retval = -ERESTARTSYS;
			break;
		}

		retval = autopoll_all_devices( board );
		if( retval < 0 )
		{
			// XXX srq may not necessarily be stuck
			board->stuck_srq = 1;
			set_bit( SRQI_NUM, &board->status );
			break;
		}
	}

	up( &board->autopoll_mutex );

	return retval;
}

static int mutex_ioctl( gpib_board_t *board, unsigned long arg )
{
	int retval, lock_mutex;

	retval = copy_from_user( &lock_mutex, ( void * ) arg, sizeof( lock_mutex ) );
	if( retval )
		return -EFAULT;

	if( lock_mutex )
	{
		retval = down_interruptible(&board->mutex);
		if(retval)
		{
			printk("gpib: ioctl interrupted while waiting on lock\n");
			return -ERESTARTSYS;
		}
	}else
	{
		up( &board->mutex );
	}

	return 0;
}
