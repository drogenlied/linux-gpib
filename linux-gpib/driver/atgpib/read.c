
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
	DBGprint(DBG_DATA, ("buf=0x%p cnt=%d  ", buf, cnt));

	GPIBout(IMR1, 0);
	GPIBout(IMR2, 0);		/* clear any previously arrived bits */

	s2 = GPIBin(ISR2);		/* clear the status registers... */
	s1 = GPIBin(ISR1) | GPIBin(ISR1);
					/* read ISR1 twice in case of END delay */

	DBGprint(DBG_DATA, ("ISR1=0x%x ISR2=0x%x  ", s1, s2));

	if (pgmstat & PS_HELD) {
		DBGprint(DBG_BRANCH, ("finish handshake  "));
		GPIBout(AUXMR, auxrabits | HR_HLDA);
		GPIBout(AUXMR, AUX_FH);	/* set HLDA in AUXRA to ensure FH works */
		pgmstat &= ~PS_HELD;
	}
	else if ((s1 & HR_DI) && (s1 & HR_END)) {
		DBGprint(DBG_BRANCH, ("one-byte read with END  "));
		GPIBout(AUXMR, auxrabits | HR_HLDA);
		pgmstat |= PS_HELD;
		buf[0] = GPIBin(DIR);
		ibsta |= END;
		ibcnt = 1;
		DBGout();
		return;
	}
	DBGprint(DBG_BRANCH, ("reset FIFO, configure TURBO  "));
	GPIBout(CMDR, RSTFIFO);		/* reset the TURBO-488's FIFO */

	DBGprint(DBG_BRANCH, ("set-up EOS modes  "));
/*
 *	Set EOS modes, holdoff on END, and holdoff on all carry cycle...
 */
	GPIBout(AUXMR, auxrabits | HR_HLDE);
	GPIBout(CCRG,  auxrabits | HR_HLDA);

	if (cnt == 1) {			/* one byte always PIO... */
	/*
	 *	The configuration register in the gate-array is configured for
	 *	- Halt GPIB transfers if the 7210 asserts its interrupt line
	 *	- Carry cycle is enabled
	 *	- BYTE (8 bit) transfers
	 */
#ifdef MARKS_BUGFIX
		GPIBout(CNTL, -(cnt+1));
		GPIBout(CNTH, -(cnt+1) >> 8);
#else
		GPIBout(CNTL, -cnt);
		GPIBout(CNTH, -cnt >> 8);
#endif
		GPIBout(CFG, C_IN | C_TLCH | C_CCEN | C_TMOE | C_T_B);
		GPIBout(CMDR, GO);
		GPIBout(IMR2, HR_DMAI);
		GPIBout(IMR1, HR_ENDIE);
	
		DBGprint(DBG_BRANCH, ("wait for single PIO byte  "));
		DBGprint(DBG_DATA, ("ISR3=0x%x  ", GPIBin(ISR3)));
		while (!(GPIBin(ISR3) & HR_NEF)) {
			if ((GPIBin(ISR3) & HR_DONE) || TimedOut())
				goto rddone;
			WaitingFor(HR_NEF | HR_DONE);
		}
		buf[ibcnt++] = GPIBin(FIFOB);
	/*
	 *	After byte received wait for done...
	 */
		while (!(GPIBin(ISR3) & HR_DONE) && NotTimedOut()) {
			DBGprint(DBG_DATA, ("ISR3=0x%x  ", GPIBin(ISR3)));
			WaitingFor(HR_DONE);
		}
rddone:
		DBGprint(DBG_BRANCH, ("done  "));
		GPIBout(CMDR, STOP);
		GPIBout(IMR1, 0);	/* clear ENDIE if set */
	}
	else {				/* DMA... */
	/*
	 *	The configuration register in the gate-array is configured for
	 *	- Halt GPIB transfers if the 7210 asserts its interrupt line
	 *	- Carry cycle is enabled
	 *	- WORD (16 bit) transfers
	 */
#ifdef MARKS_BUGFIX
		GPIBout(CNTL, -(cnt+1));
		GPIBout(CNTH, -(cnt+1) >> 8);
#else
		GPIBout(CNTL, -cnt);
		GPIBout(CNTH, -cnt >> 8);
#endif
		GPIBout(CFG, C_IN | C_TLCH | C_CCEN | C_TMOE | C_T_B | C_B16);
		DBGprint(DBG_BRANCH, ("start DMA  "));
		ibcnt = cnt - osDoDMA(rdop);
	}
	if ((s1 = GPIBin(ISR1)) & HR_END) {
		if (s1 & HR_DI) {
			DBGprint(DBG_BRANCH, ("extra byte with END  "));
			buf[ibcnt++] = GPIBin(DIR);
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

	GPIBout(IMR1, 0);
	GPIBout(IMR2, 0);		/* clear any previously arrived bits */

	s2 = GPIBin(ISR2);		/* clear the status registers... */
	s1 = GPIBin(ISR1) | GPIBin(ISR1);
					/* read ISR1 twice in case of END delay */

	DBGprint(DBG_DATA, ("ISR1=0x%x ISR2=0x%x  ", s1, s2));

	if (pgmstat & PS_HELD) {
		DBGprint(DBG_BRANCH, ("finish handshake  "));
		GPIBout(AUXMR, auxrabits | HR_HLDA);
		GPIBout(AUXMR, AUX_FH);	/* set HLDA in AUXRA to ensure FH works */
		pgmstat &= ~PS_HELD;
	}
	else if ((s1 & HR_DI) && (s1 & HR_END)) {
		DBGprint(DBG_BRANCH, ("one-byte read with END  "));
		GPIBout(AUXMR, auxrabits | HR_HLDA);
		pgmstat |= PS_HELD;
		buf[0] = GPIBin(DIR);
		ibsta |= END;
		ibcnt = 1;
		DBGout();
		return;
	}
	DBGprint(DBG_BRANCH, ("reset FIFO, configure TURBO  "));
	GPIBout(CMDR, RSTFIFO);			/* reset the TURBO-488's FIFO */

	DBGprint(DBG_BRANCH, ("set-up EOS modes  "));
/*
 *	Set EOS modes, holdoff on END, and holdoff on all carry cycle...
 */
	GPIBout(AUXMR, auxrabits | HR_HLDE);
	GPIBout(CCRG,  auxrabits | HR_HLDA);
/*
 *	The configuration register in the gate-array is configured for
 *	- Halt GPIB transfers if the 7210 asserts its interrupt line
 *	- Carry cycle is enabled
 *	- BYTE (8 bit) transfers
 */
#ifdef MARKS_BUGFIX
		GPIBout(CNTL, -(cnt+1));
		GPIBout(CNTH, -(cnt+1) >> 8);
#else
	GPIBout(CNTL, -cnt);
	GPIBout(CNTH, -cnt >> 8);
#endif
	GPIBout(CFG, C_IN | C_TLCH | C_CCEN | C_TMOE | C_T_B);
	GPIBout(CMDR, GO);
	GPIBout(IMR2, HR_DMAI);
	GPIBout(IMR1, HR_ENDIE);

	DBGprint(DBG_BRANCH, ("begin PIO loop  "));
	while (ibcnt < cnt) {
		DBGprint(DBG_DATA, ("ISR3=0x%x  (a)", GPIBin(ISR3)));
		while (!(GPIBin(ISR3) & HR_NEF)) {
			if ((GPIBin(ISR3) & HR_DONE) || TimedOut())
				goto rddone;
			WaitingFor(HR_NEF | HR_DONE);
		}
		buf[ibcnt++] = GPIBin(FIFOB);
	}
/*
	buf[ibcnt]='\0';

	DBGprint(DBG_DATA, ("buf='%s'",buf));
*/

/*
 *	If end of count then wait for done...
 */
	while (!(GPIBin(ISR3) & HR_DONE) && NotTimedOut()) {
		DBGprint(DBG_DATA, ("ISR3=0x%x  (b)", GPIBin(ISR3)));
		WaitingFor(HR_DONE);
	}
rddone:
	DBGprint(DBG_BRANCH, ("done  "));
	GPIBout(CMDR, STOP);
	GPIBout(IMR1, 0);		/* clear ENDIE if set */

	if (GPIBin(ISR1) & HR_END)
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




