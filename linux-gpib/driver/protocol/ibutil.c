#include <ibprot.h>


/*
 * IBPAD
 * change the GPIB address of the interface board.  The address
 * must be 0 through 30.  ibonl resets the address to PAD.
 */
IBLCL int ibpad(int v)
{
	DBGin("ibpad");
	if ((v < 0) || (v > 30)) {
		ibsta |= ERR;
		iberr = EARG;
	}
	else {
		myPAD = v;
		DBGprint(DBG_DATA, ("pad=0x%x  ", myPAD));
		board.primary_address( myPAD );
	}
	DBGout();
	return board.update_status();
}


/*
 * IBSAD
 * change the secondary GPIB address of the interface board.
 * The address must be 0x60 through 0x7E.  ibonl resets the
 * address to SAD.
 */
IBLCL int ibsad(int v)
{
	DBGin("ibsad");
	if (v && ((v < 0x60) || (v > 0x7F))) {
		ibsta |= ERR;
		iberr = EARG;
	}
	else {
		if (v == 0x7F)
			v = 0;		/* v == 0x7F also disables */
		if ((mySAD = v)) {
			DBGprint(DBG_BRANCH, ("enabled  "));
			board.secondary_address(mySAD - 0x60, 1);
		}
		else {
			DBGprint(DBG_BRANCH, ("disabled  "));
			board.secondary_address(0,0);
		}
	}
	DBGout();
	return board.update_status();
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
	DBGin("ibtmo");
#if 0
	if (fnInit(0) & ERR) {
		DBGout();
		return ibsta;
	}
#endif
	if ((v < TNONE) || (v > T1000s)) {
		ibsta |= ERR;
		iberr = EARG;
	}
	else {
		DBGprint(DBG_DATA, ("oldtmo=%d newtmo=%d  ", timeidx, v));
		timeidx = v;
	}
	DBGout();
	return board.update_status();
}


/*
 * IBEOT
 * Set the end-of-transmission mode for I/O operations to v.
 * If v == 1 then send EOI with the last byte of each write.
 * If v == 0 then disable the sending of EOI.
 */
IBLCL int ibeot(int v)
{
	DBGin("ibeot");
#if 0
	if (fnInit(0) & ERR) {
		DBGout();
		return ibsta;
	}
#endif
	if (v) {
		DBGprint(DBG_BRANCH, ("enable EOI  "));
		pgmstat &= ~PS_NOEOI;
	}
	else {
		DBGprint(DBG_BRANCH, ("disable EOI  "));
		pgmstat |= PS_NOEOI;
	}
	DBGout();
	return board.update_status();
}

/*
 * IBEOS
 * Set the end-of-string modes for I/O operations to v.
 *
 */
IBLCL int ibeos(int v)
{
	int ebyte, emodes;
	DBGin("ibeos");
	ebyte = v & 0xFF;
	emodes = (v >> 8) & 0xFF;
	if (emodes & ~EOSM) {
		DBGprint(DBG_BRANCH, ("bad EOS modes  "));
		ibsta |= ERR;
		iberr = EARG;
	}else 
	{
		DBGprint(DBG_DATA, ("byte=0x%x modes=0x%x  ", ebyte, emodes));
		if(emodes & REOS)
		{
			board.enable_eos(ebyte, emodes & BIN);
		}else
			board.disable_eos();
	}
	DBGout();
	return 0;
}



IBLCL int ibstat(void)
/* update the GPIB status information */
{
	return board.update_status(); 
}

