
#include <ibsys.h>

extern unsigned long timeTable[];

/*
 * Timer functions
 */
void watchdog_timeout( unsigned long arg )
/* Watchdog timeout routine */
{
	gpib_board_t *board = (gpib_board_t*) arg;
	set_bit( TIMO_NUM, &board->status );
	wake_up( &board->wait );
}

/* install timer interrupt handler */
void osStartTimer( gpib_board_t *board, int v )
/* Starts the timeout task  */
/* v = index into timeTable */
{
	clear_bit( TIMO_NUM, &board->status );

	if (v > 0)
	{
		board->timer.expires = jiffies + timeTable[ v ];   /* set number of ticks */
		board->timer.function = watchdog_timeout;
		board->timer.data = (unsigned long) board;
		add_timer( &board->timer );              /* add timer           */
	}
}


void osRemoveTimer( gpib_board_t *board )
/* Removes the timeout task */
{
	del_timer_sync( &board->timer );
}

