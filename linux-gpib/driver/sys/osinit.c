/***************************************************************************
                          osinit.c  -  description
                             -------------------

    begin                : Dec 2001
    copyright            : (C) 2001 by Frank Mori Hess
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
#include <linux/version.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/fs.h>

// early 2.4.x kernels don't define MODULE_LICENSE
#ifndef MODULE_LICENSE
#define MODULE_LICENSE(x)
#endif
MODULE_LICENSE("GPL");


/*
 * Linux initialization functions
 */
int osInit(void)
{
	return 1;
}


void osReset(void)
{
}



/*******************************************************************************
********

   Init module functions


********************************************************************************
********/

struct file_operations ib_fops =
{
	owner: THIS_MODULE,
	llseek: NULL,
	read: NULL,
	write: NULL,
	readdir: NULL,
	poll: NULL,
	ioctl: ibioctl,
	mmap: NULL,
	open: ibopen,
	flush: NULL,
	release: ibclose,
	fsync: NULL,
	fasync: NULL,
	lock: NULL,
	readv: NULL,
	writev: NULL,
// sendpage and get_unmapped_area were added in 2.4.4
//	sendpage: NULL,
//	get_unmapped_area: NULL,
};

gpib_board_t board_array[MAX_NUM_GPIB_BOARDS];

LIST_HEAD(registered_drivers);

void gpib_register_driver(gpib_interface_t *interface)
{
	list_add(&interface->list, &registered_drivers);
	printk("gpib: registered %s interface\n", interface->name);
}

void gpib_unregister_driver(gpib_interface_t *interface)
{
	int i;

	for(i = 0; i < MAX_NUM_GPIB_BOARDS; i++)
	{
		gpib_board_t *board = &board_array[i];
		if (board->interface == interface) {
			if (board->open_count > 0)
				printk("gpib: Warning:"
					" deregisted interface %s in use\n",
					interface->name);
			if (board->online > 0)
				printk("gpib: This can\'t happen:"
					" deregisted interface %s is online\n",
					interface->name);
			board->interface = NULL;
		}
	}
	list_del(&interface->list);
	printk("gpib: unregistered %s interface\n", interface->name);
}

void init_gpib_board( gpib_board_t *board )
{
	board->private_data = NULL;
	board->status = 0;
	board->ibbase = 0;
	board->ibirq = 0;
	board->ibdma = 0;
	board->master = 1;
	board->online = 0;
	board->exclusive = 0;
	board->open_count = 0;
	init_waitqueue_head(&board->wait);
	init_MUTEX(&board->mutex);
	board->locking_pid = 0;
	init_MUTEX(&board->autopoll_mutex);
	spin_lock_init(&board->spinlock);
	init_timer(&board->timer);
	board->interface = NULL;
	board->buffer_length = 0;
	board->buffer = NULL;
	INIT_LIST_HEAD( &board->device_list );
	board->pad = 0;
	board->sad = -1;
	board->usec_timeout = 3000000;
	board->autopollers = 0;
	board->stuck_srq = 0;
}

void init_board_array( gpib_board_t *board_array, unsigned int length )
{
	int i;
	for( i = 0; i < length; i++)
	{
		init_gpib_board( &board_array[i] );
	}
}

void init_gpib_device( gpib_device_t *device )
{
	INIT_LIST_HEAD( &device->list );
	INIT_LIST_HEAD( &device->status_bytes );
	device->num_status_bytes = 0;
	device->reference_count = 0;
	device->dropped_byte = 0;
}

int init_module(void)
{
	printk("Linux-GPIB Driver -- Kernel Release %s\n", UTS_RELEASE);

	init_board_array(board_array, MAX_NUM_GPIB_BOARDS);

	if(register_chrdev(IBMAJOR, "gpib", &ib_fops))
	{
		printk("can't get Major %d\n", IBMAJOR);
		return(-EIO);
	}

	return 0;
}

void cleanup_module(void)
{
	if ( unregister_chrdev(IBMAJOR, "gpib") != 0 ) {
		printk("gpib: device busy or other module error \n");
	} else {
		printk("gpib: succesfully removed \n");
	}
}

EXPORT_SYMBOL(gpib_register_driver);
EXPORT_SYMBOL(gpib_unregister_driver);
