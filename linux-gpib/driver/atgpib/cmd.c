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
	uint8		s1, s2;		/* software copies of HW status regs */

	DBGin("bdcmd");
	buf = cmdop->io_vbuf;
	cnt = cmdop->io_cnt;
	DBGprint(DBG_DATA, ("buf=0x%p cnt=%d  ", buf, cnt));

	GPIBout(imr1, 0);
	GPIBout(imr2, 0);		/* clear any previously arrived bits */

	s2 = GPIBin(isr2);		/* clear the status registers... */
	s1 = GPIBin(isr1);
	DBGprint(DBG_DATA, ("isr1=0x%x isr2=0x%x  ", s1, s2));

	DBGprint(DBG_BRANCH, ("reset FIFO, configure TURBO  "));
	GPIBout(cmdr, RSTFIFO);
/*
 *	The configuration register in the gate-array is configured for
 *	- BYTE (8 bit) transfers
 */
	GPIBout(cntl, -cnt);
	GPIBout(cnth, -cnt >> 8);
	GPIBout(cfg, C_CMD);
	GPIBout(cmdr, GO);
	GPIBout(imr2, HR_COIE);

	DBGprint(DBG_BRANCH, ("begin PIO loop  "));
	while (ibcnt < cnt) {
		DBGprint(DBG_DATA, ("isr3=0x%x  ibcnt=%d(a)", GPIBin(isr3),ibcnt));
		while (!(GPIBin(isr3) & HR_NFF)) {
			if ((GPIBin(isr3) & HR_DONE) || TimedOut())
				goto cmddone;
			WaitingFor(HR_NFF | HR_DONE);
		}
		GPIBout(fifo.f8.b, buf[ibcnt++]);
	}
	DBGprint(DBG_BRANCH, ("wait for DONE  "));
/*
 *	If end of count then wait for done...
 */
	while (!(GPIBin(isr3) & HR_DONE) && NotTimedOut()) {
		DBGprint(DBG_DATA, ("isr3=0x%x  (b)", GPIBin(isr3)));
		WaitingFor(HR_DONE);
	}
cmddone:
	if (!noTimo) {
		DBGprint(DBG_BRANCH, ("timeout  "));
		ibsta |= (ERR | TIMO);
		iberr = EABO;
	}
	DBGprint(DBG_BRANCH, ("done  "));
	GPIBout(cmdr, STOP);
	GPIBout(imr2, 0);		/* clear COIE if set */
	ibcnt = cnt + (GPIBin(cntl) | (GPIBin(cnth)<<8));
	DBGout();
}








