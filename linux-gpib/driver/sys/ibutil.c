
#include "gpibP.h"
#include "autopoll.h"
/*
 * IBPAD
 * change the GPIB address of the interface board.  The address
 * must be 0 through 30.  ibonl resets the address to PAD.
 */
int ibpad( gpib_board_t *board, unsigned int addr )
{
	if ( addr > 30 )
	{
		printk("gpib: invalid primary address %u\n", addr );
		return -1;
	}else
	{
		board->pad = addr;
		if( board->online )
			board->interface->primary_address( board, board->pad );
		GPIB_DPRINTK( "set primary addr to %i\n", board->pad );
	}
	return 0;
}


/*
 * IBSAD
 * change the secondary GPIB address of the interface board.
 * The address must be 0 through 30, or negative disables.  ibonl resets the
 * address to SAD.
 */
int ibsad( gpib_board_t *board, int addr )
{
	if( addr > 30 )
	{
		printk("gpib: invalid secondary address %i, must be 0-30\n", addr);
		return -1;
	}else
	{
		board->sad = addr;
		if( board->online )
		{
			if( board->sad >= 0 )
			{
				board->interface->secondary_address( board, board->sad, 1 );
			}else
			{
				board->interface->secondary_address( board, 0, 0 );
			}
		}
		GPIB_DPRINTK( "set secondary addr to %i\n", board->sad );
	}
	return 0;
}

/*
 * IBEOS
 * Set the end-of-string modes for I/O operations to v.
 *
 */
int ibeos( gpib_board_t *board, int eos, int eosflags )
{
	if( eosflags & ~EOS_MASK )
	{
		printk( "bad EOS modes\n" );
		return -EINVAL;
	}else
	{
		if( eosflags & REOS )
		{
			board->interface->enable_eos( board, eos, eosflags & BIN );
		}else
			board->interface->disable_eos( board );
	}
	return 0;
}

int ibstatus( gpib_board_t *board )
{
	return general_ibstatus( board, NULL, 0, 0 );
}

int general_ibstatus( gpib_board_t *board, const gpib_device_t *device,
	int clear_mask, pid_t cmpl_pid )
{
	int status = 0;

	if( board->private_data )
	{
		status = board->interface->update_status( board, clear_mask );
		/* XXX should probably stop having drivers use TIMO bit in
		 * board->status to avoid confusion */
		status &= ~TIMO;
	}
	if( device )
		if( num_status_bytes( device ) ) status |= RQS;

	if( cmpl_pid && board->locking_pid == cmpl_pid )
		status &= ~CMPL;
	else
		status |= CMPL;

	if( num_gpib_events( &board->event_queue ) )
		status |= EVENT;
	else
		status &= ~EVENT;

	return status;
}
