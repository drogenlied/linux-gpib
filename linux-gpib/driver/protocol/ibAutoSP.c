#include <ibprot.h>

/* unfortunately this in not a real protocol function */
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/major.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/timer.h>

/*
 * AP_Lock/Unlock are global locks that avoids multiple 'waiting for interrupt'-s
 * so no other process can enter the region until autopolling has been finished
 */

#define MAX_DEVICES 31

DECLARE_MUTEX(AutoP_mutex);

void AP_Lock (void)
{
  DBGin("AP_Lock");
  down(&AutoP_mutex);
  DBGprint(DBG_DATA,("AutoP_mutex=%d",atomic_read(&AutoP_mutex.count)));
  DBGout();
}

void AP_UnLock (void)
{
  DBGin("AP_UnLock");
  up(&AutoP_mutex);
  DBGprint(DBG_DATA,("AutoP_mutex=%d",atomic_read(&AutoP_mutex.count)));
  DBGout();
}


/*
 * The device entry in the Poll vector
 *
 */

struct AP_device {
  struct semaphore lock;
  int    flags;
  char   spb;
  int    stat;
};

/* Flags */

#define AP_POLL    (1<<0)            /* Poll this device next time */
#define AP_BUSY    (1<<1)            /* Serial poll is running     */
#define AP_PENDING (1<<2)            /* Device has been polled but no ibrsp */

struct AP_device AP_Vector[MAX_DEVICES];  /* The Poll Vector (only one entry per PAD) :-/ */

/*
 * Initialize the Poll Vector
 *
 */

void AP_Init(void)
{
  int i;
  DBGin("AP_Init");
  for(i=0;i<MAX_DEVICES;i++) {
    init_MUTEX(&AP_Vector[i].lock);
   /*(AP_Vector[i].lock).count = 1;*/
    AP_Vector[i].flags= 0;
    AP_Vector[i].spb  = 0;
  }
  DBGout();
}

/*
 * Device Locking (only one process can wait for RQS)
 */

void AP_LocalLock(int pad)
{
  DBGin("AP_LocalLock");
  down(&(AP_Vector[pad].lock));
  DBGprint(DBG_DATA,("Vector[%d].lock=%d",pad,atomic_read(&(AP_Vector[pad].lock).count)));
  DBGout();
}

void AP_LocalUnLock(int pad)
{
  DBGin("AP_LocalUnLock");
  up(&(AP_Vector[pad].lock));
  DBGprint(DBG_DATA,("Vector[%d].lock=%d",pad,atomic_read(&(AP_Vector[pad].lock).count)));
  DBGout();
}

/*
 * ibAPWait() ... the game goes on
 *
 */

int AP_virgin = 1;

IBLCL int ibAPWait(gpib_device_t *device, int pad)
{
  int i;

  DBGin("ibAPWait");
  pad &=0xff;

  DBGprint(DBG_DATA,("Waiting on Device %d",pad));

  /* Init the poll vector if not done so */
     if( AP_virgin )
       { AP_Init(); AP_virgin=0; }

  /* Lock device and set poll stat */
     AP_Vector[pad].flags |= AP_POLL;
     AP_LocalLock(pad);

  /* Enter IRQ region */
     AP_Lock();

  /* if RQS has been returned in the last cycle return without requesting
     for interrupt */
     if( (AP_Vector[pad].flags & AP_PENDING) && (AP_Vector[pad].spb & RQS) ){
       AP_UnLock();
       AP_LocalUnLock(pad);
       /*FIXME: there is no state that can be returned here*/
       DBGout();
       return ibstatus(device);
     }

  /* wait for SRQ interrupt */
     if( ibwait(device, SRQI) & ERR ){
       AP_UnLock();
       AP_LocalUnLock(pad);
       DBGout();
       return ibstatus(device);
     }

  /* poll all devices with AP_POLL set */
	//XXX
     if(down_interruptible(&device->mutex))
	printk("interrupted waiting on lock in ibAPWait\n");

     for(i=0;i<MAX_DEVICES;i++){
       if( AP_Vector[i].flags & AP_POLL ){
	 dvrsp(device, i,&(AP_Vector[i].spb ));
	 AP_Vector[i].stat = ibstatus(device);
	 AP_Vector[i].flags  |= AP_PENDING;
         DBGprint(DBG_DATA,("device %d -> spb=0x%x",i,AP_Vector[i].spb));
       }
     }
     up(&device->mutex);
     if(! (AP_Vector[pad].spb & 0x40 ) ) {
       printk("Ouups: No RQS after Autopoll Operation ?\n");
     }

  /* Exit IRQ Region */
     AP_UnLock();
  /* Unlock device */
     AP_LocalUnLock(pad);
     DBGout();
     return ibstatus(device);
}

/*
 * This routine reads out the serial poll response with
 * autopolling
 *
 */

IBLCL int ibAPrsp(gpib_device_t *device, int padsad, char *spb)
{
        int pad = padsad & 0xff;

	DBGin("ibAPrsp");
        /* lock poll operation */
	AP_Lock();
        if(!(AP_Vector[pad].flags & AP_PENDING) ) {
           printk("Device %d not in AP_PENDING state?\n",pad);
	   DBGout();
           /* fall back to normal serial poll */
	   return dvrsp(device, padsad,spb);
        }

        *spb = AP_Vector[pad].spb;
        AP_Vector[pad].flags &= ~AP_PENDING;
//	device->status =  AP_Vector[pad].stat;

        AP_UnLock();
	DBGout();

	return ibstatus(device); 	/* 980728 TBg */
}


/*
 * ibAPE(int padsad, int v)
 * controls AP_POLL in poll vector
 *
 *
 */

IBLCL void ibAPE(gpib_device_t *device, int pad, int v)
{
  DBGin("ibAPE");

  pad &= 0xff;
  DBGprint(DBG_DATA,("AP_POLL(%d)=%d",pad,v));
  if(v){
    AP_Vector[pad].flags |= AP_POLL;
  } else {
    AP_Vector[pad].flags &= ~AP_POLL;
  }
  DBGout();
}
