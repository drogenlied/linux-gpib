
#include "gpibP.h"
#include <linux/delay.h>

/*
 * IBSRE
 * Send REN true if v is non-zero or false if v is zero.
 */
int ibsre(gpib_board_t *board, int enable)
{
	board->interface->remote_enable(board, enable);	/* set or clear REN */
	if(!enable)
	{
		udelay(100);
	}

	return 0;
}

