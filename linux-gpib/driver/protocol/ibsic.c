#include <ibprot.h>
#include <linux/delay.h>

extern int drvstat,ib_opened;

/*
 * IBSIC
 * Send IFC for at least 100 microseconds.
 *
 * NOTE:
 *      1.  Ibsic must be called prior to the first call to
 *          ibcmd in order to initialize the bus and enable the
 *          interface to leave the controller idle state.
 */
int ibsic(gpib_board_t *board)
{
	if(!test_bit(CIC_NUM, &board->status))
	{

		board->master = 1;
		/* set controller state */
		board->interface->interface_clear(board, 1);                   /* assert IFC */
		udelay(100);
		board->interface->interface_clear(board, 0);                   /* clear IFC */
	}
// CIC should be set by driver, this is a temporary hack for tms9914 XXX
	set_bit(CIC_NUM, &board->status);

	return 0;
}

