
#include "gpibP.h"
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/module.h>

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
		board->buffer = vmalloc( board->buffer_length );
		if(board->buffer == NULL)
			return -ENOMEM;

		if( board->interface->attach( board ) < 0 )
		{
			board->interface->detach(board);
			printk("GPIB Hardware Error! (Chip type not found or wrong Base Address?)\n");
			return -1;
		}

		if( master )
		{
			board->master = 1;
			ibsic( board );
		}else
		{
			board->master = 0;
		}
	}

	__MOD_INC_USE_COUNT( board->interface->provider_module );
	board->online++;

	return 0;
}

// XXX need to make sure autopoll is not in progress
int iboffline( gpib_board_t *board )
{
	if( board->online == 0 )
	{
		return 0;
	}

	if( board->online == 1 )
	{
		board->interface->detach( board );
		if( board->buffer )
		{
			vfree( board->buffer );
			board->buffer = NULL;
		}
	}
	__MOD_DEC_USE_COUNT( board->interface->provider_module );
	board->online--;

	return 0;
}

