#include <ibsys.h>


struct timer_list ibtimer_list;



#if SYSTIMO
/*
 * Timer functions
 */
IBLCL void ibtmintr(unsigned long unused)			
/* Watchdog timeout routine */
{
#if DEBUG
	if (dbgMask & DBG_INTR)
		printk("TIMER INTERRUPT!\n");
#endif
	noTimo = 0;
#if USEINTS
	if (!(pgmstat & PS_NOINTS)) {

#ifdef NIAT
		GPIBout(imr3, 0);	                /* disable interrupts */
#endif
		up(&espsemid);
	}
#endif
}
#endif

/* install timer interrupt handler */


IBLCL void osStartTimer(int v)			
                                        /* Starts the timeout task  */
					/* v = index into timeTable */
{
	DBGin("osStartTimer");
	noTimo = INITTIMO;
#if USEINTS
	if (!(pgmstat & PS_NOINTS))
  #ifdef LINUX2_2
                sema_init( &espsemid, 0);
  #else
	        espsemid.count = 0;
  #endif
#endif
	if (v > 0) {
		DBGprint(DBG_DATA, ("timo=%d  ", timeTable[v]));
#if SYSTIMO
#ifdef LINUX2_0
		ibtimer_list.expires = jiffies + timeTable[v];   /* set number of ticks */
#else
		ibtimer_list.expires = timeTable[v];   /* set number of ticks */
#endif
		ibtimer_list.function = ibtmintr;
		/*ibtimer_list.data    = (long) espwdid;*/
		add_timer(&ibtimer_list);              /* add timer           */
		pgmstat |= PS_TIMINST;                 /* mark watchdog installed */
#else
		noTimo = timeTable[v];
		pgmstat |= PS_TIMINST;
#endif
	}
	DBGout();
}


IBLCL void osRemoveTimer(void)
			/* Removes the timeout task */
{
	DBGin("osRemoveTimer");
	if (pgmstat & PS_TIMINST) {
#if SYSTIMO
		del_timer(&ibtimer_list);
#else
		if (noTimo > 0)
			noTimo = INITTIMO;
#endif
		pgmstat &= ~PS_TIMINST;  /* unmark watchdog */
	}
	DBGout();
}

