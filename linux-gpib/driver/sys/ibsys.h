#include <gpibP.h>

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/major.h>
#define __NO_VERSION__
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/timer.h>

#include <asm/io.h>
#include <asm/segment.h>
#include <asm/irq.h>
#include <asm/dma.h>
#include <asm/uaccess.h>
#include <asm/system.h>

#include <board.h>

#ifndef IBMAJOR
/* default value for major number */
#define IBMAJOR 250
#endif

#define LINUX_VERS	1
#define LINUX_PATCH	51

#define DMA_PAGE_SIZE (128*1024)       /* Page Boundaries for channel 5-7 transfers */
#define MAX_DMA_MEMORY (16*1024*1024)  /* DMA transfers limited to lower 16M */

#define MAX_DMA_SIZE	(64 * 1024)	/* Maximum DMA transfer size 	*/

#define TICKSPERSEC 100
#define OK 0

extern int pgmstat;
extern volatile int noTimo;
extern uint32_t timeTable[];

extern struct wait_queue *ibwait_queue;

extern struct semaphore espsemid;		/* semaphore ID for ESP interrupt support */

extern int   gpib_dma_size;
extern int   gpib_default_dma_size;


