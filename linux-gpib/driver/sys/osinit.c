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

// early 2.4.x kernels don't define MODULE_LICENSE
#ifndef MODULE_LICENSE
#define MODULE_LICENSE(x)
#endif
MODULE_LICENSE("GPL");

MODULE_PARM(ibmajor, "i");
MODULE_PARM_DESC(ibmajor, "major device number of gpib device file");
MODULE_PARM(dbgMask, "i");
MODULE_PARM_DESC(dbgMask, "controls amount of debugging information the module dumps");

/* default debugging level */

#if DEBUG
#ifndef DEFAULT_DEBUG
unsigned int   dbgMask  = (DBG_ENTRY | DBG_EXIT | DBG_BRANCH | DBG_DATA | DBG_INTR | DBG_1PPL) & ~DBG_ALL;
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

ibboard_t ibboard   = { 0 };	/* driver info, etc */

/*
 * LINUX specific stuff
 * 
 * 
 */

struct wait_queue *ibwait_queue = NULL;



/*
 * Linux initialization functions
 */
IBLCL int osInit(void)
{
	int	s;
extern  struct timer_list ibtimer_list;

	DBGin("osInit");

#if SYSTIMO
	s = TICKSPERSEC;
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
	ibtimer_list.list.next = ibtimer_list.list.prev = NULL;
#endif

#if USEINTS
        sema_init( &espsemid, 0);
	DBGprint(DBG_DATA, ("espsemid=0x%x  ", atomic_read(&espsemid.count)));
#endif
	pgmstat |= PS_SYSRDY;
	DBGout();
	return 1;
}


IBLCL void osReset(void)
{
	DBGin("osReset");
	pgmstat &= ~PS_SYSRDY;
	DBGout();
}



/***************************************************************************************

   Init module functions


****************************************************************************************/

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


#ifdef __cplusplus
extern "C" {
#endif

int ibmajor = IBMAJOR;   /* major number for dynamic configuration */


int init_module(void)
{
	EXPORT_NO_SYMBOLS;
#if DEBUG
	if(dbgMask != 0 )
         	dbgMask |= DBG_1PPL;
#endif

        printk("Linux-GPIB Driver Board=%s -- Major=%d ",BOARD_TYPE,ibmajor);
#if !defined(CBI_PCMCIA) && !defined(INES_PCMCIA)
        printk("Base=0x%lx ",ibbase );
#if USEINTS
	printk("Irq=%d ",ibirq );
#endif
#endif
#if DMAOP
	printk("DMA=%d enabled\n",ibdma);
#else
	printk("DMA disabled\n");
#endif
#if DEBUG
	printk("-- DebugMask = 0x%x\n",dbgMask);
#endif
	printk("-- Kernel Release %s\n", UTS_RELEASE);

  	DBGin("ibinstall");
	
	if( register_chrdev(ibmajor,"gpib",&ib_fops)){
	  printk("can't get Major %d\n",ibmajor);
          DBGout();
	  return(-EIO);
	}
	osMemInit();
	DBGout();
	return 0;

}

void cleanup_module(void)
{
  DBGin("cleanup");

  if (MOD_IN_USE) {
    printk("gpib: device busy, remove delayed\n");
  }
  else {
    board_detach();
  }

  osMemRelease();

  if ( unregister_chrdev(ibmajor, "gpib") != 0 ) {
    printk("gpib: device busy or other module error \n");
  } else {
    printk("gpib: succesfully removed \n");
  }
  DBGout();
}



#ifdef __cplusplus
}
#endif

