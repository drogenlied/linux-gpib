
#include "gpibP.h"
#include <linux/kernel.h>
#include <linux/vmalloc.h>

/*
* IBONL
* Initialize the interface hardware.  If v is non-zero then
* the GPIB chip is enabled online.  If v is zero then it is
* left disabled and offline.
*
* NOTE:
*      1.  Ibonl must be called before any other function.
*/

int ibonline( gpib_board_t *board, int master )
{
	if( !board->online )
	{
		board->buffer_length = 0x1000;
		if( board->buffer )
			vfree( board->buffer );
		board->buffer = vmalloc( board->buffer_length );
		if(board->buffer == NULL)
			return -ENOMEM;

		if( board->interface->attach( board ) < 0 )
		{
			board->interface->detach(board);
			printk("GPIB Hardware Error! (Chip type not found or wrong Base Address?)\n");
			return -1;
		}
		/* initialize system support functions */
		board->online = 1;

		if( master )
		{
			board->master = 1;
			ibsic( board );
		}else
		{
			board->master = 0;
		}
	}

	return 0;
}

int iboffline( gpib_board_t *board )
{
	if( board->open_count <= 1 && board->online )
	{
		board->interface->detach( board );
		if( board->buffer )
		{
			vfree( board->buffer );
		}
		init_gpib_board( board );
	}
	return 0;
}

