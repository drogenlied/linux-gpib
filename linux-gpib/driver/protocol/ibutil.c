#include <ibprot.h>

/*
 * IBPAD
 * change the GPIB address of the interface board.  The address
 * must be 0 through 30.  ibonl resets the address to PAD.
 */
IBLCL int ibpad(int v)
{
	if ((v < 0) || (v > 30))
	{
		printk("gpib: invalid primary address\n");
		return -1;
	}else
	{
		myPAD = v;
		driver->primary_address(driver, myPAD );
	}
	return 0;
}


/*
 * IBSAD
 * change the secondary GPIB address of the interface board.
 * The address must be 0x60 through 0x7E.  ibonl resets the
 * address to SAD.
 */
IBLCL int ibsad(int v)
{
	if (v && ((v < 0x60) || (v > 0x7F)))
	{
		printk("gpib: invalid secondary address\n");
		return -1;
	}else
	{
		if (v == 0x7F)
			v = 0;		/* v == 0x7F also disables */
		if ((mySAD = v))
		{
			driver->secondary_address(driver, mySAD - 0x60, 1);
		}else
		{
			driver->secondary_address(driver, 0,0);
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
IBLCL int ibtmo(int v)
{
	if ((v < TNONE) || (v > T1000s))
	{
		printk("gpib: error setting timeout\n");
		return -1;
	}else
	{
		timeidx = v;
	}
	return 0;
}


/*
 * IBEOT
 * Set the end-of-transmission mode for I/O operations to v.
 * If v == 1 then send EOI with the last byte of each write.
 * If v == 0 then disable the sending of EOI.
 */
IBLCL int ibeot(int send_eoi)
{
	if (send_eoi)
	{
		pgmstat &= ~PS_NOEOI;
	}else
	{
		pgmstat |= PS_NOEOI;
	}
	return 0;
}

/*
 * IBEOS
 * Set the end-of-string modes for I/O operations to v.
 *
 */
IBLCL int ibeos(int v)
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
			driver->enable_eos(driver, ebyte, emodes & BIN);
		}else
			driver->disable_eos(driver);
	}
	return 0;
}

IBLCL unsigned int ibstatus()
{
	if(driver->private_data == NULL)
		return 0;

	return driver->update_status(driver);
}
