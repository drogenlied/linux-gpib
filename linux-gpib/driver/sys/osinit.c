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

// early 2.4.x kernels don't define MODULE_LICENSE
#ifndef MODULE_LICENSE
#define MODULE_LICENSE(x)
#endif
MODULE_LICENSE("GPL");

MODULE_PARM(ibmajor, "i");
MODULE_PARM_DESC(ibmajor, "major device number of gpib device file");
MODULE_PARM(dbgMask, "i");
MODULE_PARM_DESC(dbgMask, "controls amount of debugging information the module
 dumps");

/* default debugging level */

#if DEBUG
#ifndef DEFAULT_DEBUG
unsigned int   dbgMask  = (DBG_ENTRY | DBG_EXIT | DBG_BRANCH | DBG_DATA |
 DBG_INTR | DBG_1PPL) & ~DBG_ALL;
#else
unsigned int   dbgMask  = 0;
#endif
#endif


/* static variables for debugging */

int   fidx     = 0;		/* index into function name stack		*/
char *fstk[32] =			/* function name stack (NO OVERFLOW PROTECTION)	*/
{
	"NULL"
};

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
IBLCL int osInit(void)
{
	int	s;
extern  struct timer_list ibtimer_list;

	DBGin("osInit");

	s = HZ;
	DBGprint(DBG_DATA, ("ClkRate=%d  ", s));
	if (s != timeTable[0]) {
		DBGprint(DBG_BRANCH, ("adjusting timeTable %d", s));
		timeTable[ 0] = s;			/* (New TMFAC)  */
		timeTable[ 1] = TM(s,10,1000000L);	/*  1: T10us    */
		timeTable[ 2] = TM(s,30,1000000L);	/*  2: T30us    */
		timeTable[ 3] = TM(s,100,1000000L);	/*  3: T100us   */
		timeTable[ 4] = TM(s,300,1000000L);	/*  4: T300us   */
		timeTable[ 5] = TM(s,1,1000);		/*  5: T1ms     */
		timeTable[ 6] = TM(s,3,1000);		/*  6: T3ms     */
		timeTable[ 7] = TM(s,10,1000);		/*  7: T10ms    */
		timeTable[ 8] = TM(s,30,1000);		/*  8: T30ms    */
		timeTable[ 9] = TM(s,100,1000);		/*  9: T100ms   */
		timeTable[10] = TM(s,300,1000);		/* 10: T300ms   */
		timeTable[11] = TM(s,1,1);		/* 11: T1s      */
		timeTable[12] = TM(s,3,1);		/* 12: T3s      */
		timeTable[13] = TM(s,10,1);		/* 13: T10s     */
		timeTable[14] = TM(s,30,1);		/* 14: T30s     */
		timeTable[15] = TM(s,100,1);		/* 15: T100s    */
		timeTable[16] = TM(s,300,1);		/* 16: T300s    */
		timeTable[17] = TM(s,1000,1);		/* 17: T1000s   */
	}
	init_timer(&ibtimer_list);

	pgmstat |= PS_SYSRDY;
	DBGout();
	return 1;
}


IBLCL void osReset(void)
{
	pgmstat &= ~PS_SYSRDY;
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
	sendpage: NULL,
	get_unmapped_area: NULL,
};

int ibmajor = IBMAJOR;   /* major number for dynamic configuration */

gpib_device_t device_array[MAX_NUM_GPIB_DEVICES];

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

int init_module(void)
{
	printk("Linux-GPIB Driver Board=%s -- Major=%d\n", BOARD_TYPE, ibmajor);
	printk("-- Kernel Release %s\n", UTS_RELEASE);

	if(register_chrdev(ibmajor, "gpib", &ib_fops))
	{
		printk("can't get Major %d\n",ibmajor);
		return(-EIO);
	}
	osMemInit();

	return 0;
}

void cleanup_module(void)
{

	osMemRelease();

	if ( unregister_chrdev(ibmajor, "gpib") != 0 ) {
		printk("gpib: device busy or other module error \n");
	} else {
		printk("gpib: succesfully removed \n");
	}
}

EXPORT_SYMBOL(gpib_register_driver);
EXPORT_SYMBOL(gpib_unregister_driver);
