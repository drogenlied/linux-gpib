
#include "gpibP.h"
#include <linux/kernel.h>
#include <linux/vmalloc.h>

#if !defined(HP82335) && !defined(TMS9914)
#else
int ccrbits	= 0;	/* static bits for AUXRA (EOS modes) */
#endif

/*
* IBONL
* Initialize the interface hardware.  If v is non-zero then
* the GPIB chip is enabled online.  If v is zero then it is
* left disabled and offline.
*
* NOTE:
*      1.  Ibonl must be called before any other function.
*/

int drvstat = 0;

int ibonline( gpib_board_t *board )
{
	if( ( board->open_count <= 1 ) && !( board->online ) )
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
	}
	/* initialize system support functions */
	board->online = 1;
	if( board->master )
		ibsic(board);
	return 0;
}

int iboffline( gpib_board_t *board )
{
	if( board->open_count <= 1 )
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

