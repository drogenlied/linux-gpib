/***************************************************************************
                               sys/ibinit.c
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
#include <linux/vmalloc.h>
#include <linux/smp_lock.h>

static int autospoll_thread(void *board_void)
{
	gpib_board_t *board = board_void;
	int retval = 0;

	lock_kernel();
	/* This thread doesn't need any user-level access,
	 * so get rid of all our resources..
	 */
	daemonize();
	/* set our name for identification purposes */
	sprintf(current->comm, "gpib_autospoll");
	unlock_kernel();

	GPIB_DPRINTK("entering autospoll thread\n" );

	while(1)
	{
		if(wait_event_interruptible(board->wait,
			board->master && board->autospollers > 0 &&
			board->stuck_srq == 0 &&
			test_and_clear_bit(SRQI_NUM, &board->status)))
		{
			retval = -ERESTARTSYS;
			break;
		}

		if(down_interruptible(&board->autopoll_mutex))
		{
			return -ERESTARTSYS;
		}
		/* make sure autopolling is still on now after we have
		 * lock */
		if(board->autospollers <= 0)
		{
			up(&board->autopoll_mutex);
			continue;
		}
		retval = autopoll_all_devices(board);
		if(retval <= 0)
		{
			board->stuck_srq = 1;	// XXX could be better
			set_bit(SRQI_NUM, &board->status);
		}
		up(&board->autopoll_mutex);
	}

	GPIB_DPRINTK("exiting autospoll thread\n" );
	return retval;
}

int ibonline( gpib_board_t *board )
{
	int retval;

	if( board->online ) return -EBUSY;

	board->autospoll_pid = kernel_thread(autospoll_thread, board, 0);
	if(board->autospoll_pid < 0)
	{
		printk("gpib: failed to create autospoll thread\n");
		return board->autospoll_pid;
	}
	retval = gpib_allocate_board( board );
	if( retval < 0 ) return retval;

	if( board->interface->attach( board ) < 0 )
	{
		board->interface->detach(board);
		printk("gpib: interface attach failed\n");
		return -1;
	}
	board->online = 1;
	GPIB_DPRINTK( "gpib: board online\n" );

	return 0;
}

/* XXX need to make sure autopoll is not in progress,
 * and board is generaly not in use (grab board lock?) */
int iboffline( gpib_board_t *board )
{
	int retval;

	if( board->online == 0 )
	{
		return 0;
	}

	board->interface->detach( board );
	gpib_deallocate_board( board );
	if(board->autospoll_pid <= 0)
		printk("gpib: bug! autospoll_pid is not positive\n");
	else
	{
		retval = kill_proc(board->autospoll_pid, SIGKILL, 1);
		if(retval)
			printk("gpib: kill_proc returned %i\n", retval);
	}
	board->online = 0;
	GPIB_DPRINTK( "gpib: board offline\n" );

	return 0;
}

