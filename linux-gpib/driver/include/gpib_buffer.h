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

#define GPIB_MAX_BUFFER_SIZE 4096

// holds a character of data, along with where EOI or EOS characters were received
typedef struct gpib_char_struct
{
	uint8_t value;	// actual data
	uint8_t end;	// boolean value that marks end of gpib string
} gpib_char_t;

// buffer for storing incoming/outgoing data
typedef struct gpib_buffer_struct
{
	gpib_char_t array[GPIB_MAX_BUFFER_SIZE];
	gpib_char_t *front, *back;
	atomic_t size;	// number of elements currently stored in buffer
	spinlock_t lock;
} gpib_buffer_t;

// dynamically initialize buffer
extern inline void gpib_buffer_init(gpib_buffer_t *buffer)
{
	buffer->front = buffer->back = buffer->array;
	atomic_set(&buffer->size, 0);
	spin_lock_init(&buffer->lock);
};

// put element into fifo
extern inline int gpib_buffer_put(gpib_buffer_t *buffer, gpib_char_t data)
{
	if(atomic_read(&buffer->size) >= GPIB_MAX_BUFFER_SIZE)
	{
		return -1;
	}
	*buffer->back = data;
	buffer->back++;
	if(buffer->back >= buffer->array + GPIB_MAX_BUFFER_SIZE)
		buffer->back = buffer->array;
	atomic_inc(&buffer->size);
	return 0;
};

// get element from fifo
extern inline int gpib_buffer_get(gpib_buffer_t *buffer, gpib_char_t *data)
{
	if(atomic_read(&buffer->size) == 0)
	{
		return -1;
	}
	atomic_dec(&buffer->size);
	*data = *buffer->front;
	buffer->front++;
	if(buffer->front >= buffer->array + GPIB_MAX_BUFFER_SIZE)
		buffer->front = buffer->array;
	return 0;
};

#endif	// _GPIB_BUFFER_H

