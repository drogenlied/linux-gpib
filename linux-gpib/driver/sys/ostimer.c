#include <ibsys.h>

extern unsigned long timeTable[];

/*
 * Timer functions
 */
void watchdog_timeout(unsigned long arg)
/* Watchdog timeout routine */
{
	gpib_device_t *device = (gpib_device_t*) arg;
	set_bit(TIMO_NUM, &device->status);
	wake_up(&device->wait);
}

/* install timer interrupt handler */
void osStartTimer(gpib_device_t *device, int v)
/* Starts the timeout task  */
/* v = index into timeTable */
{
	clear_bit(TIMO_NUM, &device->status);

	if (v > 0)
	{
		device->timer.expires = jiffies + timeTable[v];   /* set number of ticks */
		device->timer.function = watchdog_timeout;
		device->timer.data = (unsigned long) device;
		add_timer(&device->timer);              /* add timer           */
	}
}


void osRemoveTimer(gpib_device_t *device)
/* Removes the timeout task */
{
	if(timer_pending(&device->timer))
	{
		del_timer_sync(&device->timer);
	}
}

