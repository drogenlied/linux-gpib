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

/* format patterns for different function levels 
 * so debugging output is easier to read
 * this will cost a little memory of course but there is 
 * not much perfomance drop */

char *ffmt[32] = 
{
   "",
   " ",
   "  ",
   "   ",
   "    ",
   "     ",
   "      ",
   "       ",
   "        ",
   "         ",
   "          ",
   "           ",
   "            ",
   "             ",
   "              ",
   "               ",
   "*",
   "* ",
   "*  ",
   "*   ",
   "*    ",
   "*     ",
   "*      ",
   "*       ",
   "*        ",
   "*         ",
   "*          ",
   "*           ",
   "*            ",
   "*             ",
   "*              ",
   "*               "
};


#if SYSTIMO
int       espwdid   = 0;		/* watchdog timer ID for ESP routines */
#endif
#if USEINTS
struct semaphore espsemid;		/* semaphore ID for ESP interrupt support */
int       espintcon = 0;		        /* ESP interrupt routine is "connected" */
#endif

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
	list_del(&interface->list);
	printk("gpib: unregistered %s interface\n", interface->name);
}

void init_board_array(gpib_board_t *board_array, unsigned int length)
{
	int i;
	for( i = 0; i < length; i++)
	{
		board_array[i].private_data = NULL;
		board_array[i].status = 0;
		board_array[i].ibbase = 0;
		board_array[i].ibirq = 0;
		board_array[i].ibdma = 0;
		board_array[i].master = 1;
		board_array[i].online = 0;
		init_waitqueue_head(&board_array[i].wait);
		init_MUTEX(&board_array[i].mutex);
		spin_lock_init(&board_array[i].spinlock);
		init_timer(&board_array[i].timer);
		board_array[i].interface = NULL;
		board_array[i].buffer_length = 0;
		board_array[i].buffer = NULL;
	}
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
