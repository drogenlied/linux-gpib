/***************************************************************************
                          gpib_buffer.h  -  description
                             -------------------
	SMP and interrupt (semi)safe buffer for gpib read/writes

    begin                : Fri Jan 11 2002
    copyright            : (C) 2002 by Frank Mori Hess
    email                : fmhess@users.soruceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GPIB_BUFFER_H
#define _GPIB_BUFFER_H

#include <linux/spinlock.h>
#include <asm/atomic.h>

// buffer for storing incoming/outgoing data
typedef struct gpib_buffer_struct
{
	uint8_t *data;	// data buffer
	size_t length;	// length of data buffer
	uint8_t *front, *back;
	atomic_t size;	// number of elements currently stored in buffer
	/* flag that indicates END has been received during read or
	 * EOI should be sent at end of write */
	volatile unsigned int end_flag : 1;
	/* flag errors */
	volatile unsigned int error_flag : 1;
} gpib_buffer_t;

// dynamically initialize buffer
extern inline void init_gpib_buffer(gpib_buffer_t *buffer, uint8_t *data, size_t length)
{
	buffer->data = data;
	buffer->length = length;
	buffer->front = buffer->back = buffer->data;
	atomic_set(&buffer->size, 0);
	buffer->end_flag = 0;
	buffer->error_flag = 0;
};

// put element into fifo
extern inline int gpib_buffer_put(gpib_buffer_t *buffer, uint8_t data)
{
	if(atomic_read(&buffer->size) >= buffer->length)
	{
		return -1;
	}
	*buffer->back = data;
	buffer->back++;
	if(buffer->back >= buffer->data + buffer->length)
		buffer->back = buffer->data;
	atomic_inc(&buffer->size);
	return 0;
};

// get element from fifo
extern inline int gpib_buffer_get(gpib_buffer_t *buffer, uint8_t *data)
{
	if(atomic_read(&buffer->size) == 0)
	{
		return -1;
	}
	*data = *buffer->front;
	buffer->front++;
	if(buffer->front >= buffer->data + buffer->length)
		buffer->front = buffer->data;
	atomic_dec(&buffer->size);
	return 0;
};

#endif	// _GPIB_BUFFER_H

