
#include <board.h>

/*
 *  BDREAD (DMA)
 *  This function performs a single DMA read operation.
 *  Note that the hand-shake is held off at the end of every read.
 */
#if DMAOP
IBLCL void bdDMAread(ibio_op_t *rdop)
{
	faddr_t		buf;
	unsigned	cnt;
	uint8		s1, s2;		/* software copies of HW status regs */

	DBGin("bdread(dma)");
	buf = rdop->io_vbuf;
	cnt = rdop->io_cnt;
	DBGprint(DBG_DATA, ("buf=0x%x cnt=%d  ", buf, cnt));

	GPIBout(imr1, 0);
	GPIBout(imr2, 0);		/* clear any previously arrived bits */

	s2 = GPIBin(isr2);		/* clear the status registers... */
	s1 = GPIBin(isr1) | GPIBin(isr1);
					/* read isr1 twice in case of END delay */

	DBGprint(DBG_DATA, ("isr1=0x%x isr2=0x%x  ", s1, s2));

	if (pgmstat & PS_HELD) {
		DBGprint(DBG_BRANCH, ("finish handshake  "));
		GPIBout(auxmr, auxrabits | HR_HLDA);
		GPIBout(auxmr, AUX_FH);	/* set HLDA in AUXRA to ensure FH works */
		pgmstat &= ~PS_HELD;
	}
	else if ((s1 & HR_DI) && (s1 & HR_END)) {
		DBGprint(DBG_BRANCH, ("one-byte read with END  "));
		GPIBout(auxmr, auxrabits | HR_HLDA);
		pgmstat |= PS_HELD;
		buf[0] = GPIBin(dir);
		ibsta |= END;
		ibcnt = 1;
		DBGout();
		return;
	}
	DBGprint(DBG_BRANCH, ("reset FIFO, configure TURBO  "));
	GPIBout(cmdr, RSTFIFO);		/* reset the TURBO-488's FIFO */

	DBGprint(DBG_BRANCH, ("set-up EOS modes  "));
/*
 *	Set EOS modes, holdoff on END, and holdoff on all carry cycle...
 */
	GPIBout(auxmr, auxrabits | HR_HLDE);
	GPIBout(ccrg,  auxrabits | HR_HLDA);

	if (cnt == 1) {			/* one byte always PIO... */
	/*
	 *	The configuration register in the gate-array is configured for
	 *	- Halt GPIB transfers if the 7210 asserts its interrupt line
	 *	- Carry cycle is enabled
	 *	- BYTE (8 bit) transfers
	 */
#ifdef MARKS_BUGFIX
		GPIBout(cntl, -(cnt+1));
		GPIBout(cnth, -(cnt+1) >> 8);
#else
		GPIBout(cntl, -cnt);
		GPIBout(cnth, -cnt >> 8);
#endif
		GPIBout(cfg, C_IN | C_TLCH | C_CCEN | C_TMOE | C_T_B);
		GPIBout(cmdr, GO);
		GPIBout(imr2, HR_DMAI);
		GPIBout(imr1, HR_ENDIE);
	
		DBGprint(DBG_BRANCH, ("wait for single PIO byte  "));
		DBGprint(DBG_DATA, ("isr3=0x%x  ", GPIBin(isr3)));
		while (!(GPIBin(isr3) & HR_NEF)) {
			if ((GPIBin(isr3) & HR_DONE) || TimedOut())
				goto rddone;
			WaitingFor(HR_NEF | HR_DONE);
		}
		buf[ibcnt++] = GPIBin(fifo.f8.b);
	/*
	 *	After byte received wait for done...
	 */
		while (!(GPIBin(isr3) & HR_DONE) && NotTimedOut()) {
			DBGprint(DBG_DATA, ("isr3=0x%x  ", GPIBin(isr3)));
			WaitingFor(HR_DONE);
		}
rddone:
		DBGprint(DBG_BRANCH, ("done  "));
		GPIBout(cmdr, STOP);
		GPIBout(imr1, 0);	/* clear ENDIE if set */
	}
	else {				/* DMA... */
	/*
	 *	The configuration register in the gate-array is configured for
	 *	- Halt GPIB transfers if the 7210 asserts its interrupt line
	 *	- Carry cycle is enabled
	 *	- WORD (16 bit) transfers
	 */
#ifdef MARKS_BUGFIX
		GPIBout(cntl, -(cnt+1));
		GPIBout(cnth, -(cnt+1) >> 8);
#else
		GPIBout(cntl, -cnt);
		GPIBout(cnth, -cnt >> 8);
#endif
		GPIBout(cfg, C_IN | C_TLCH | C_CCEN | C_TMOE | C_T_B | C_B16);
		DBGprint(DBG_BRANCH, ("start DMA  "));
		ibcnt = cnt - osDoDMA(rdop);
	}
	if ((s1 = GPIBin(isr1)) & HR_END) {
		if (s1 & HR_DI) {
			DBGprint(DBG_BRANCH, ("extra byte with END  "));
			buf[ibcnt++] = GPIBin(dir);
		}
		ibsta |= END;
	}
	if ((ibsta & END) || (ibcnt == cnt))
		pgmstat |= PS_HELD;
	if (!(ibsta & ERR) && !noTimo) {
		DBGprint(DBG_BRANCH, ("timeout  "));
		ibsta |= (ERR | TIMO);
		iberr = EABO;
	}
	DBGout();
}



#else

/*
 *  BDREAD (PIO)
 *  This function performs a single Programmed I/O read operation.
 *  Note that the hand-shake is held off at the end of every read.
 */
IBLCL void bdPIOread(ibio_op_t *rdop)
{
	faddr_t		buf;
	unsigned	cnt;
	uint8		s1, s2;		/* software copies of HW status regs */

	DBGin("bdread");
	buf = rdop->io_vbuf;
	cnt = rdop->io_cnt;
	DBGprint(DBG_DATA, ("buf=0x%x cnt=%d  ", buf, cnt));

	GPIBout(imr1, 0);
	GPIBout(imr2, 0);		/* clear any previously arrived bits */

	s2 = GPIBin(isr2);		/* clear the status registers... */
	s1 = GPIBin(isr1) | GPIBin(isr1);
					/* read isr1 twice in case of END delay */

	DBGprint(DBG_DATA, ("isr1=0x%x isr2=0x%x  ", s1, s2));

	if (pgmstat & PS_HELD) {
		DBGprint(DBG_BRANCH, ("finish handshake  "));
		GPIBout(auxmr, auxrabits | HR_HLDA);
		GPIBout(auxmr, AUX_FH);	/* set HLDA in AUXRA to ensure FH works */
		pgmstat &= ~PS_HELD;
	}
	else if ((s1 & HR_DI) && (s1 & HR_END)) {
		DBGprint(DBG_BRANCH, ("one-byte read with END  "));
		GPIBout(auxmr, auxrabits | HR_HLDA);
		pgmstat |= PS_HELD;
		buf[0] = GPIBin(dir);
		ibsta |= END;
		ibcnt = 1;
		DBGout();
		return;
	}
	DBGprint(DBG_BRANCH, ("reset FIFO, configure TURBO  "));
	GPIBout(cmdr, RSTFIFO);			/* reset the TURBO-488's FIFO */

	DBGprint(DBG_BRANCH, ("set-up EOS modes  "));
/*
 *	Set EOS modes, holdoff on END, and holdoff on all carry cycle...
 */
	GPIBout(auxmr, auxrabits | HR_HLDE);
	GPIBout(ccrg,  auxrabits | HR_HLDA);
/*
 *	The configuration register in the gate-array is configured for
 *	- Halt GPIB transfers if the 7210 asserts its interrupt line
 *	- Carry cycle is enabled
 *	- BYTE (8 bit) transfers
 */
#ifdef MARKS_BUGFIX
		GPIBout(cntl, -(cnt+1));
		GPIBout(cnth, -(cnt+1) >> 8);
#else
	GPIBout(cntl, -cnt);
	GPIBout(cnth, -cnt >> 8);
#endif
	GPIBout(cfg, C_IN | C_TLCH | C_CCEN | C_TMOE | C_T_B);
	GPIBout(cmdr, GO);
	GPIBout(imr2, HR_DMAI);
	GPIBout(imr1, HR_ENDIE);

	DBGprint(DBG_BRANCH, ("begin PIO loop  "));
	while (ibcnt < cnt) {
		DBGprint(DBG_DATA, ("isr3=0x%x  (a)", GPIBin(isr3)));
		while (!(GPIBin(isr3) & HR_NEF)) {
			if ((GPIBin(isr3) & HR_DONE) || TimedOut())
				goto rddone;
			WaitingFor(HR_NEF | HR_DONE);
		}
		buf[ibcnt++] = GPIBin(fifo.f8.b);
	}
/*
	buf[ibcnt]='\0';

	DBGprint(DBG_DATA, ("buf='%s'",buf));
*/

/*
 *	If end of count then wait for done...
 */
	while (!(GPIBin(isr3) & HR_DONE) && NotTimedOut()) {
		DBGprint(DBG_DATA, ("isr3=0x%x  (b)", GPIBin(isr3)));
		WaitingFor(HR_DONE);
	}
rddone:
	DBGprint(DBG_BRANCH, ("done  "));
	GPIBout(cmdr, STOP);
	GPIBout(imr1, 0);		/* clear ENDIE if set */

	if (GPIBin(isr1) & HR_END)
		ibsta |= END;
	if ((ibsta & END) || (ibcnt == cnt))
		pgmstat |= PS_HELD;
	if (!noTimo) {
		DBGprint(DBG_BRANCH, ("timeout  "));
		ibsta |= (ERR | TIMO);
		iberr = EABO;
	}
	DBGout();
}

#endif




