#include <ibprot.h>


/*
 * IBPAD
 * change the GPIB address of the interface board.  The address
 * must be 0 through 30.  ibonl resets the address to PAD.
 */
IBLCL int ibpad(int v)
{
	DBGin("ibpad");
	if (fnInit(0) & ERR) {
		DBGout();
		return ibsta;
	}
	if ((v < 0) || (v > 30)) {
		ibsta |= ERR;
		iberr = EARG;
	}
	else {
		myPAD = v;
		DBGprint(DBG_DATA, ("pad=0x%x  ", myPAD));
		/*GPIBout(adr, (myPAD & LOMASK));*/
		bdSetPAD( myPAD );
	}
	ibstat();
	DBGout();
	return ibsta;
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
	if (fnInit(0) & ERR) {
		DBGout();
		return ibsta;
	}
	if (v && ((v < 0x60) || (v > 0x7F))) {
		ibsta |= ERR;
		iberr = EARG;
	}
	else {
		if (v == 0x7F)
			v = 0;		/* v == 0x7F also disables */
		if ((mySAD = v)) {
			DBGprint(DBG_BRANCH, ("enabled  "));
			bdSetSAD(mySAD,1);
		}
		else {
			DBGprint(DBG_BRANCH, ("disabled  "));
			bdSetSAD(mySAD,0);
		}
	}
	ibstat();
	DBGout();
	return ibsta;
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
	if (fnInit(0) & ERR) {
		DBGout();
		return ibsta;
	}
	if ((v < TNONE) || (v > T1000s)) {
		ibsta |= ERR;
		iberr = EARG;
	}
	else {
		DBGprint(DBG_DATA, ("oldtmo=%d newtmo=%d  ", timeidx, v));
		timeidx = v;
	}
	ibstat();
	DBGout();
	return ibsta;
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
	if (fnInit(0) & ERR) {
		DBGout();
		return ibsta;
	}
	if (v) {
		DBGprint(DBG_BRANCH, ("enable EOI  "));
		pgmstat &= ~PS_NOEOI;
	}
	else {
		DBGprint(DBG_BRANCH, ("disable EOI  "));
		pgmstat |= PS_NOEOI;
	}
	ibstat();
	DBGout();
	return ibsta;
}


/*
 * IBEOS
 * Set the end-of-string modes for I/O operations to v.
 * 
 */
IBLCL int ibeos(int v)
{
	int ebyte, emodes;
#if defined(HP82335) || defined(NIPCII) || defined(TMS9914)
	extern int eosmodes;
#endif
	DBGin("ibeos");
	if (fnInit(0) & ERR) {
		DBGout();
		return ibsta;
	}
	ebyte = v & 0xFF;
	emodes = (v >> 8) & 0xFF;
	if (emodes & ~EOSM) {
		DBGprint(DBG_BRANCH, ("bad EOS modes  "));
		ibsta |= ERR;
		iberr = EARG;
	}
	else {
#if !defined(HP82335) && !defined(TMS9914)
		auxrabits = AUXRA | emodes;
#endif
#if defined(HP82335) || defined(TMS9914) || defined(NIPCII)
		eosmodes = emodes;
#endif
		DBGprint(DBG_DATA, ("byte=0x%x modes=0x%x  ", ebyte, emodes));
		bdSetEOS(ebyte);
		bdSendAuxACmd(0);
	}
	ibstat();
	DBGout();
	return ibsta;
}



IBLCL int ibstat(void)			
/* update the GPIB status information */
{
	register int brdstat;

	DBGin("ibstat");
	brdstat = bdGetAdrStat();
	ibsta |= bdSRQstat();
	ibsta |= (brdstat & HR_CIC)  ? CIC  : 0;
#if !defined(HP82335) && !defined(TMS9914)
	ibsta |= (brdstat & HR_NATN) ? 0    : ATN;
#else
	ibsta |= (brdstat & HR_ATN) ? ATN    : 0;
#endif
	ibsta |= (brdstat & HR_TA)   ? TACS : 0;
	ibsta |= (brdstat & HR_LA)   ? LACS : 0;
	DBGprint(DBG_DATA, ("ibsta=0x%x  ", ibsta));
	DBGout();
	return ibsta;
}

