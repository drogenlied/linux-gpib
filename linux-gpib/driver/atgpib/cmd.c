#include <board.h>

/*
 *  BDCMD
 *  This function performs a single Programmed I/O command operation.
 *  Note that ATN must already be asserted when this function is called.
 */
IBLCL void bdcmd(ibio_op_t *cmdop)
{
	faddr_t		buf;
	unsigned	cnt;
	uint8_t		s1, s2;		/* software copies of HW status regs */

	DBGin("bdcmd");
	buf = cmdop->io_vbuf;
	cnt = cmdop->io_cnt;
	DBGprint(DBG_DATA, ("buf=0x%p cnt=%d  ", buf, cnt));

	GPIBout(IMR1, 0);
	GPIBout(IMR2, 0);		/* clear any previously arrived bits */

	s2 = GPIBin(ISR2);		/* clear the status registers... */
	s1 = GPIBin(ISR1);
	DBGprint(DBG_DATA, ("ISR1=0x%x ISR2=0x%x  ", s1, s2));

	DBGprint(DBG_BRANCH, ("reset FIFO, configure TURBO  "));
	GPIBout(CMDR, RSTFIFO);
/*
 *	The configuration register in the gate-array is configured for
 *	- BYTE (8 bit) transfers
 */
	GPIBout(CNTL, -cnt);
	GPIBout(CNTH, -cnt >> 8);
	GPIBout(CFG, C_CMD);
	GPIBout(CMDR, GO);
	GPIBout(IMR2, HR_COIE);

	DBGprint(DBG_BRANCH, ("begin PIO loop  "));
	while (ibcnt < cnt) {
		DBGprint(DBG_DATA, ("ISR3=0x%x  ibcnt=%d(a)", GPIBin(ISR3),ibcnt));
		while (!(GPIBin(ISR3) & HR_NFF)) {
			if ((GPIBin(ISR3) & HR_DONE) || TimedOut())
				goto cmddone;
			WaitingFor(HR_NFF | HR_DONE);
		}
		GPIBout(FIFOB, buf[ibcnt++]);
	}
	DBGprint(DBG_BRANCH, ("wait for DONE  "));
/*
 *	If end of count then wait for done...
 */
	while (!(GPIBin(ISR3) & HR_DONE) && NotTimedOut()) {
		DBGprint(DBG_DATA, ("ISR3=0x%x  (b)", GPIBin(ISR3)));
		WaitingFor(HR_DONE);
	}
cmddone:
	if (!noTimo) {
		DBGprint(DBG_BRANCH, ("timeout  "));
		ibsta |= (ERR | TIMO);
		iberr = EABO;
	}
	DBGprint(DBG_BRANCH, ("done  "));
	GPIBout(CMDR, STOP);
	GPIBout(IMR2, 0);		/* clear COIE if set */
	ibcnt = cnt + (GPIBin(CNTL) | (GPIBin(CNTH)<<8));
	DBGout();
}








