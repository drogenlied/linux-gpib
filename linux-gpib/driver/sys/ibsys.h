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



#ifndef IBMAJOR
/* default value for major number */
#define IBMAJOR 31
#endif

#define LINUX_VERS	1
#define LINUX_PATCH	51


 #define DMA_PAGE_SIZE (128*1024)       /* Page Boundaries for channel 5-7 transfers */
 #define MAX_DMA_MEMORY (16*1024*1024)  /* DMA transfers limited to lower 16M */

 #ifdef CONFIG_DMA_MEM_PERM
  #define MAX_DMA_SIZE	(128*1024)	/* Maximum DMA transfer size 	*/
 #else
  #define MAX_DMA_SIZE	(63*1024)	/* Maximum DMA transfer size 	*/
 #endif


#define TICKSPERSEC 100
#define OK 0



extern unsigned long      ibbase;	/* base addr of GPIB interface registers  */
extern uint8       ibirq;	/* interrupt request line for GPIB (1-7)  */
extern uint8       ibdma ;      /* DMA channel                            */

extern int pgmstat;
extern volatile int noTimo;
extern uint32 timeTable[];


extern struct wait_queue *ibwait_queue;


extern struct semaphore espsemid;		/* semaphore ID for ESP interrupt support */


extern int   gpib_dma_size;
extern int   gpib_default_dma_size;

#define IB ib              




