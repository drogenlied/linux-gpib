#include <ibprot.h>

/*
 * IBPAD
 * change the GPIB address of the interface board.  The address
 * must be 0 through 30.  ibonl resets the address to PAD.
 */
IBLCL int ibpad(gpib_device_t *device, int v)
{
	if ((v < 0) || (v > 30))
	{
		printk("gpib: invalid primary address\n");
		return -1;
	}else
	{
		myPAD = v;
		device->interface->primary_address(device, myPAD );
	}
	return 0;
}


/*
 * IBSAD
 * change the secondary GPIB address of the interface board.
 * The address must be 0x60 through 0x7E.  ibonl resets the
 * address to SAD.
 */
IBLCL int ibsad(gpib_device_t *device, int v)
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
			device->interface->secondary_address(device, mySAD - 0x60, 1);
		}else
		{
			device->interface->secondary_address(device, 0,0);
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
IBLCL int ibtmo(gpib_device_t *device, int v)
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
IBLCL int ibeot(gpib_device_t *device, int send_eoi)
{
	if(send_eoi)
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
IBLCL int ibeos(gpib_device_t *device, int v)
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
			device->interface->enable_eos(device, ebyte, emodes & BIN);
		}else
			device->interface->disable_eos(device);
	}
	return 0;
}

IBLCL unsigned int ibstatus(gpib_device_t *device)
{
	if(device->private_data == NULL)
		return 0;

	return device->interface->update_status(device);
}
