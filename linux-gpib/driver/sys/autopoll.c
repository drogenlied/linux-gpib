/***************************************************************************
                               sys/autopoll.c
                             -------------------

    copyright            : (C) 2002 by Frank Mori Hess
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

unsigned int num_status_bytes( const gpib_device_t *dev )
{
	return dev->num_status_bytes;
}

// push status byte onto back of status byte fifo
int push_status_byte( gpib_device_t *device, uint8_t poll_byte )
{
	struct list_head *head = &device->status_bytes;
	status_byte_t *status;

	status = kmalloc( sizeof( status_byte_t ), GFP_KERNEL );
	if( status == NULL ) return -ENOMEM;

	INIT_LIST_HEAD( &status->list );
	status->poll_byte = poll_byte;

	list_add_tail( &status->list, head );

	device->num_status_bytes++;

	GPIB_DPRINTK( "pushed status byte 0x%x, %i in queue\n",
		(int) poll_byte, device->num_status_bytes );

	return 0;
}

// pop status byte from front of status byte fifo
int pop_status_byte( gpib_device_t *device, uint8_t *poll_byte )
{
	struct list_head *head = &device->status_bytes;
	struct list_head *front = head->next;
	status_byte_t *status;

	if( device->num_status_bytes == 0 ) return -1;

	if( front == head ) return -1;

	status = list_entry( front, status_byte_t, list );
	*poll_byte = status->poll_byte;

	list_del( front );
	kfree( status );

	device->num_status_bytes--;

	GPIB_DPRINTK( "popped status byte 0x%x, %i in queue\n",
		(int) *poll_byte, device->num_status_bytes );

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
		if( gpib_address_equal( device->pad, device->sad, pad, sad ) )
			return device;
	}

	return NULL;
}

int get_serial_poll_byte( gpib_board_t *board, unsigned int pad, int sad, unsigned int usec_timeout,
		uint8_t *poll_byte )
{
	gpib_device_t *device;

	device = get_gpib_device( board, pad, sad );
	if( device == NULL ) return -EINVAL;

	if( num_status_bytes( device ) )
	{
		return pop_status_byte( device, poll_byte );
	}else
	{
		return dvrsp( board, pad, sad, usec_timeout, poll_byte );
	}
}

int autopoll_device( gpib_board_t *board, gpib_device_t *device )
{
	static const unsigned int serial_timeout = 1000000;
	static const unsigned int max_num_status_bytes = 1024;
	uint8_t poll_byte;
	int retval;
	static const uint8_t request_service_bit = 0x40;

	if( num_status_bytes( device ) >= max_num_status_bytes )
	{
		uint8_t lost_byte;

		device->dropped_byte = 1;
		retval = pop_status_byte( device, &lost_byte );
		if( retval < 0 ) return retval;
	}

	retval = dvrsp( board, device->pad, device->sad, serial_timeout, &poll_byte );
	if( retval < 0 ) return retval;

	if( poll_byte & request_service_bit )
	{
		retval = push_status_byte( device, poll_byte );
		if( retval < 0 ) return retval;
	}

	return 0;
}

int autopoll_all_devices( gpib_board_t *board )
{
	int retval;
	struct list_head *cur;
	const struct list_head *head = &board->device_list;
	gpib_device_t *device;

	for( cur = head->next; cur != head; cur = cur->next )
	{
		if( down_interruptible( &board->mutex ) )
		{
			return -ERESTARTSYS;
		}

		device = list_entry( cur, gpib_device_t, list );
		retval = autopoll_device( board, device );
		if( retval < 0 )
		{
			up( &board->mutex );
			return retval;
		}

		up( &board->mutex );
		/* need to wake wait queue in case someone is
		 * waiting on RQS */
		wake_up_interruptible( &board->wait );
		if( current->need_resched )
			schedule();
	}

	return 0;
}
