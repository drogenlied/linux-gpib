#include <ibprot.h>

/*
 * IBPAD
 * change the GPIB address of the interface board.  The address
 * must be 0 through 30.  ibonl resets the address to PAD.
 */
int ibpad(gpib_board_t *board, int v)
{
	if ((v < 0) || (v > 30))
	{
		printk("gpib: invalid primary address\n");
		return -1;
	}else
	{
		myPAD = v;
		board->interface->primary_address(board, myPAD );
	}
	return 0;
}


/*
 * IBSAD
 * change the secondary GPIB address of the interface board.
 * The address must be 0 through 30, or negative disables.  ibonl resets the
 * address to SAD.
 */
int ibsad(gpib_board_t *board, int v)
{
	if (v > 30)
	{
		printk("gpib: invalid secondary address, must be 0-30\n");
		return -1;
	}else
	{
		if ((mySAD = v) >= 0) // mySAD shouldn't be global XXX
		{
			board->interface->secondary_address(board, mySAD, 1);
		}else
		{
			board->interface->secondary_address(board, 0,0);
		}
	}
	return 0;
}


/*
 * IBTMO
 * Set the timeout value for I/O operations to v.  Timeout
 * intervals can range from 10 usec to 1000 sec.  The value
 * of v specifies an index into the array timeTable.
 * If v == 0 then timeouts are disabled.
 */
int ibtmo(gpib_board_t *board, unsigned int timeout)
{
	if (timeout > T1000s)
	{
		printk("gpib: invalid timeout setting\n");
		return -EINVAL;
	}else
	{
		timeidx = timeout;	// XXX global
	}
	return 0;
}


/*
 * IBEOT
 * Set the end-of-transmission mode for I/O operations to v.
 * If v == 1 then send EOI with the last byte of each write.
 * If v == 0 then disable the sending of EOI.
 */
int ibeot(gpib_board_t *board, int send_eoi)
{
	board->send_eoi = send_eoi;
	return 0;
}

/*
 * IBEOS
 * Set the end-of-string modes for I/O operations to v.
 *
 */
int ibeos(gpib_board_t *board, int v)
{
	int ebyte, emodes;
	ebyte = v & 0xFF;
	emodes = (v >> 8) & 0xFF;
	if (emodes & ~EOSM)
	{
		printk("bad EOS modes\n");
		return -1;
	}else
	{
		if(emodes & REOS)
		{
			board->interface->enable_eos(board, ebyte, emodes & BIN);
		}else
			board->interface->disable_eos(board);
	}
	return 0;
}

unsigned int ibstatus(gpib_board_t *board)
{
	if(board->private_data == NULL)
		return 0;

	return board->interface->update_status(board);
}
