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
 * end of a successful serial poll the response is returned in result.
 * SPD and UNT are sent at the completion of the poll.
 */


#if  defined(HP82335) || defined(TMS9914)

#define HR_HLDA AUX_HLDA
#define HR_DI   HR_BI

#endif

IBLCL int dvrsp(int padsad, uint8_t *result)
{
	char cmd_string[2];
	uint8_t isreg1;
	int sp_noTimo;
	int status = board.update_status();
	int end_flag;
	ssize_t ret;

	DBGin("dvrsp");
	if((status & CIC) == 0)
	{
		DBGout();
		return status;
	}
	if (receive_setup(padsad))
	{
		// send serial poll command
		osStartTimer(pollTimeidx);

		cmd_string[0] = SPE;
		ibcmd(cmd_string, 1);

		ret = board.read(result, 1, &end_flag);
		if(ret <= 0)
		{
			printk("gpib serial poll failed\n");
			*result = 0;
		}

		cmd_string[0] = SPD;	/* disable serial poll bytes */
		cmd_string[1] = UNT;
		if ((ibcmd(cmd_string, 2) < 0) || !noTimo)
		{
			DBGprint(DBG_DATA, ("ps_noTimo=%d  ", noTimo));
			ibsta |= ERR;	/* something went wrong */
			iberr = EBUS;
		}
		osRemoveTimer();
	}
	DBGout();

	return status;
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
IBLCL int receive_setup(int padsad)
{
	uint8_t pad, sad;
	char cmdString[8];
	unsigned int i = 0;

	pad = padsad;
	sad = padsad >> 8;
	if ((pad > 0x1E) || (sad && ((sad < 0x60) || (sad > 0x7E)))) {
		printk("gpib bad addr");
		iberr = EARG;
		return -1;
	}

	cmdString[i++] = UNL;

	cmdString[i++] = myPAD | LAD;	/* controller's listen address */
	if (mySAD)
		cmdString[i++] = mySAD;
	cmdString[i++] = pad | TAD;
	if (sad)
		cmdString[i++] = sad;

	if ((ibcmd(cmdString, i) & ERR) && (iberr == EABO))
		iberr = EBUS;
	return 0;
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


