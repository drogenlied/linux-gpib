#include <ibsys.h>
#include <board.h>

struct timer_list ibtimer_list;

/*
 * Timer functions
 */
IBLCL void watchdog_timeout(unsigned long arg)
/* Watchdog timeout routine */
{
	gpib_driver_t *driver = (gpib_driver_t*) arg;
	set_bit(TIMO_NUM, &driver->status);
	wake_up(&driver->wait);
}

/* install timer interrupt handler */
IBLCL void osStartTimer(int v)
/* Starts the timeout task  */
/* v = index into timeTable */
{
	clear_bit(TIMO_NUM, &driver->status);

	if (v > 0)
	{
		ibtimer_list.expires = jiffies + timeTable[v];   /* set number of ticks */
		ibtimer_list.function = watchdog_timeout;
		ibtimer_list.data = (unsigned long) driver;
		add_timer(&ibtimer_list);              /* add timer           */
		pgmstat |= PS_TIMINST;                 /* mark watchdog installed */
	}
}


IBLCL void osRemoveTimer(void)
/* Removes the timeout task */
{
	if (pgmstat & PS_TIMINST)
	{
		del_timer_sync(&ibtimer_list);
		pgmstat &= ~PS_TIMINST;  /* unmark watchdog */
	}
}

