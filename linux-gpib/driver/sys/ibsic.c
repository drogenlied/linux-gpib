
#include "gpibP.h"
#include <linux/delay.h>

/*
 * IBSIC
 * Send IFC for at least 100 microseconds.
 *
 * NOTE:
 *      1.  Ibsic must be called prior to the first call to
 *          ibcmd in order to initialize the bus and enable the
 *          interface to leave the controller idle state.
 */
int ibsic( gpib_board_t *board, unsigned int usec_duration )
{
	if( usec_duration < 100 ) usec_duration = 100;
	if( usec_duration > 1000 ) 
	{
		usec_duration = 1000;
		printk( "gpib: warning, shortening long udelay\n");
	}

	GPIB_DPRINTK( "sending interface clear\n" );
	board->master = 1;
	board->interface->interface_clear(board, 1);
	udelay( usec_duration );
	board->interface->interface_clear(board, 0);

	return 0;
}

