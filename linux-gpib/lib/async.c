/***************************************************************************
                          lib/async.c
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

#include "ib_internal.h"
#include <sys/ioctl.h>
#include <pthread.h>

void init_async_op( struct async_operation *async )
{
	pthread_mutex_init( &async->lock, NULL );
	async->buffer = NULL;
	async->buffer_length = 0;
	async->in_progress = 0;
	async->ibsta = 0;
	async->ibcntl = 0;
	async->iberr = 0;
}
