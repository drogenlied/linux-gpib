#include <ibprot.h>


/*
 * DVTRG
 * Trigger the device with primary address pad and secondary 
 * address sad.  If the device has no secondary address, pass a
 * zero in for this argument.  This function sends the TAD of the
 * GPIB interface, UNL, the LAD of the device, and GET.
 */ 
IBLCL int dvtrg(int padsad)
{
	char cmdString[2];
	int status = board.update_status();

	DBGin("dvtrg");
	if((status & CIC) == 0)
	{
		DBGout();
		return status;
	}
	if (!(send_setup(padsad) & ERR)) {
		cmdString[0] = GET;
		ibcmd((faddr_t)cmdString, 1);
	}
	DBGout();
	return ibsta;
}	


/*
 * DVCLR
 * Clear the device with primary address pad and secondary 
 * address sad.  If the device has no secondary address, pass a
 * zero in for this argument.  This function sends the TAD of the
 * GPIB interface, UNL, the LAD of the device, and SDC.
 */ 
IBLCL int dvclr(int padsad)
{
	char cmdString[2];
	int status = board.update_status();

	DBGin("dvclr");
	if((status & CIC ) == 0)
	{
		DBGout();
		return status;
	}
	if (!(send_setup(padsad) & ERR)) {
		cmdString[0] = SDC;
		ibcmd((faddr_t)cmdString, 1);
	}
	DBGout();
	return ibsta;
}	


/*
 * DVRSP
 * This function performs a serial poll of the device with primary 
 * address pad and secondary address sad. If the device has no
 * secondary adddress, pass a zero in for this argument.  At the
 * end of a successful serial poll the response is returned in spb.
 * SPD and UNT are sent at the completion of the poll.
 */


#if  defined(HP82335) || defined(TMS9914)

#define HR_HLDA AUX_HLDA
#define HR_DI   HR_BI

#endif

IBLCL int dvrsp(int padsad,char *spb)
{
#if 0
	char spdString[3];
	uint8_t isreg1;
	int sp_noTimo;
	
	DBGin("dvrsp");
	if (fnInit(HR_CIC) & ERR) {
		DBGout();
		return ibsta;
	}
	if (!(receive_setup(padsad, 1) & ERR)) {
        	spdString[0] = SPD;	/* disable serial poll bytes */
        	spdString[1] = UNT;

		bdSendAuxACmd(HR_HLDA);
		if (pgmstat & PS_HELD) {
			DBGprint(DBG_BRANCH, ("finish handshake  "));
			bdSendAuxCmd(AUX_FH);
			pgmstat &= ~PS_HELD;
		}
		board.go_to_standby();
	
		osStartTimer(pollTimeidx);
		DBGprint(DBG_BRANCH, ("wait for spoll byte  "));
		isreg1 = bdWaitIn();
					/* wait for byte to come in */
		bdSendAuxCmd(AUX_TCS);
		if (isreg1 & HR_DI) {
			DBGprint(DBG_BRANCH, ("received  "));
			*spb = bdGetDataByte();
			DBGprint(DBG_DATA, ("spb=0x%x  ", *spb));
			pgmstat |= PS_HELD;
		}
		else {
			DBGprint(DBG_BRANCH, ("NOT received  "));
			*spb = 0;
		}
		bdWaitATN();
					/* wait for synchronous take control */
		sp_noTimo = noTimo;
		osRemoveTimer();
		if ((ibcmd((faddr_t)spdString, 2) & ERR) || !sp_noTimo) {
		        DBGprint(DBG_DATA, ("ps_noTimo=%d  ", sp_noTimo));
			ibsta |= ERR;	/* something went wrong */
			iberr = EBUS;
		}
		else	ibcnt = 1;	/* SUCCESS: spoll byte received */

	}
	DBGout();
#endif
	return ibsta;
}


/*
 * DVRD
 * Read cnt bytes from the device with primary address pad and
 * secondary address sad.  If the device has no secondary address,
 * pass a zero in for this argument.  The device is adressed to
 * talk and the GPIB interface is addressed to listen.  ibrd is
 * then called to read cnt bytes from the device and place them
 * in buf.
 */
IBLCL ssize_t dvrd(int padsad,faddr_t buf,unsigned int cnt)
{
	ssize_t ret = 0;
	int status = board.update_status();
	DBGin("dvrd");

	ibcnt = 0;
	if((status & CIC) == 0) 
	{
		printk("board not CIC on dvrd\n");
		DBGout();
		return -1;
	}
	if (receive_setup(padsad, 0) & ERR)
		return -1;

	ret = ibrd(buf, cnt);

	DBGout();
	return ret;
}


/*
 * DVWRT
 * Write cnt bytes to the device with primary address pad and
 * secondary address sad.  If the device has no secondary address,
 * pass a zero in for this argument.  The device is adressed to
 * listen and the GPIB interface is addressed to talk.  ibwrt is
 * then called to write cnt bytes to the device from buf.
 */
IBLCL int dvwrt(int padsad,faddr_t buf,unsigned int cnt)
{
	int status = board.update_status();
	DBGin("dvwrt");
	ibcnt = 0;
	if((status & CIC) == 0) 
	{
		DBGout();
		return status;
	}
	if (!(send_setup(padsad) & ERR)){
		ibwrt(buf, cnt, 0);	// XXX assumes all the data is written in this call
	}
	DBGout();
	return ibsta;
}


/*
 * 488.2 Controller sequences
 */
IBLCL int receive_setup(int padsad,int spoll)

				/* spoll = TRUE if this is for a serial poll */
{
	uint8_t pad, sad;
	char cmdString[8];
	unsigned int i = 0;

	DBGin("Rsetup");
	pad = padsad;
	sad = padsad >> 8;
	DBGprint(DBG_DATA, ("pad=0x%x, sad=0x%x  ", pad, sad));
	if ((pad > 0x1E) || (sad && ((sad < 0x60) || (sad > 0x7E)))) {
		DBGprint(DBG_BRANCH, ("bad addr  "));
		ibsta |= ERR;
		iberr = EARG;
		ibstat();
		DBGout();
		return ibsta;
	}

	cmdString[i++] = UNL;

	cmdString[i++] = myPAD | LAD;	/* controller's listen address */
	if (mySAD)
		cmdString[i++] = mySAD;
	if (spoll) {
		DBGprint(DBG_BRANCH, ("SPE  "));
		cmdString[i++] = SPE;
	}
	cmdString[i++] = pad | TAD;
	if (sad)
		cmdString[i++] = sad;

	if ((ibcmd(cmdString, i) & ERR) && (iberr == EABO))
		iberr = EBUS;
	DBGout();
	return ibsta;
}


IBLCL int send_setup(int padsad)
{
	uint8_t pad, sad;
	char cmdString[8];
	unsigned i = 0;

	DBGin("Ssetup");
	pad = padsad;
	sad = padsad >> 8;
	DBGprint(DBG_DATA, ("pad=0x%x, sad=0x%x  ", pad, sad));
	if ((pad > 0x1E) || (sad && ((sad < 0x60) || (sad > 0x7E)))) {
		DBGprint(DBG_BRANCH, ("bad addr  "));
		ibsta |= ERR;
		iberr = EARG;
		ibstat();
		DBGout();
		return ibsta;
	}

	cmdString[i++] = UNL;
	cmdString[i++] = pad | LAD;
	if (sad)
		cmdString[i++] = sad;
	cmdString[i++] = myPAD | TAD;	/* controller's talk address */
	if (mySAD)
		cmdString[i++] = mySAD;

	if ((ibcmd(cmdString, i) & ERR) && (iberr == EABO))
		iberr = EBUS;


	DBGout();
	return ibsta;
}


