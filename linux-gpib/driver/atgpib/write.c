
#include <board.h>

/* @BEGIN-MAN

\routine{  bdwrt(wrtop) ibio_op_t *wrtop  }
\synopsis{  }
\description{  This function performs a single write operation. }
\returns{   }
\bugs{   }

   @END-MAN */

#if DMAOP
/*
 *  BDWRT (DMA)
 *  This function performs a single DMA write operation.
 */
IBLCL void bdDMAwrt(ibio_op_t *wrtop)
{ 
	faddr_t		buf;
	unsigned	cnt;
	uint8		s1, s2;		/* software copies of HW status regs */

	DBGin("bdwrt(dma)");
	buf = wrtop->io_vbuf;
	cnt = wrtop->io_cnt;
	DBGprint(DBG_DATA, ("buf=0x%p cnt=%d  ", buf, cnt));

	GPIBout(IMR1, 0);
	GPIBout(IMR2, 0);		/* clear any previously arrived bits */

	s2 = GPIBin(ISR2);		/* clear the status registers... */
	s1 = GPIBin(ISR1);
	DBGprint(DBG_DATA, ("ISR1=0x%x ISR2=0x%x  ", s1, s2));

	DBGprint(DBG_BRANCH, ("reset FIFO, configure TURBO  "));
	GPIBout(CMDR, RSTFIFO);

	DBGprint(DBG_BRANCH, ("set-up EOT and EOS  "));
/*
 *	Set EOT and EOS modes...
 */
	if (!(wrtop->io_flags & IO_LAST) || (pgmstat & PS_NOEOI)) {
		DBGprint(DBG_BRANCH, ("no EOI  "));
		wrtop->io_ccfunc = 0;	/* no default EOI w/last byte */
	}
	else {
		DBGprint(DBG_BRANCH, ("send EOI on carry-cycle  "));
		wrtop->io_ccfunc = AUX_SEOI;
	}				/* send EOI w/last byte */

	GPIBout(AUXMR, auxrabits);	/* send EOI w/EOS if requested */

	if (cnt == 1) {			/* one byte always PIO... */
	/*
	 *	The configuration register in the gate-array is configured for
	 *	- Halt GPIB transfers if the 7210 asserts its interrupt line
	 *	- Carry cycle is enabled
	 *	- BYTE (8 bit) transfers
	 */
		if (wrtop->io_ccfunc) {
			GPIBout(CCRG, wrtop->io_ccfunc);
			GPIBout(CFG, (C_TLCH | C_T_B | C_CCEN));
		}
		else	GPIBout(CFG, (C_TLCH | C_T_B));

		GPIBout(CNTL, -cnt);
		GPIBout(CNTH, -cnt >> 8);
		GPIBout(CMDR, GO);
		GPIBout(IMR1, HR_ERRIE);/* set IMR1 before IMR2 on write */
		GPIBout(IMR2, HR_DMAO);
	
		DBGprint(DBG_BRANCH, ("send out single PIO byte  "));
		DBGprint(DBG_DATA, ("ISR3=0x%x  ", GPIBin(ISR3)));
	/*
	 *	After "RSTFIFO" above, FIFO should be empty, so assume
	 *	HR_NFF is set.  After writing out the byte, wait for
	 *	ENOL, done, or timeout.
	 */
		GPIBout(FIFOB, buf[ibcnt++]);
		while (!(GPIBin(ISR3) & (HR_TLCI | HR_DONE)) && NotTimedOut()) {
		/*
		 *	NAT4882: If HR_TLCI is set, assume HR_ERR is
		 *	set in ISR1 because of a "no listeners" error.
		 */
			DBGprint(DBG_DATA, ("ISR3=0x%x  ", GPIBin(ISR3)));
			WaitingFor(HR_TLCI | HR_DONE);
		}
		DBGprint(DBG_BRANCH, ("done  "));
		GPIBout(CMDR, STOP);
		GPIBout(IMR1, 0);	/* clear ERRIE if set */
		if (GPIBin(CNTL))
			ibcnt--;	/* if residual count, byte not sent */
	}
	else {				/* DMA... */
	/*
	 *	The configuration register in the gate-array is configured for
	 *	- Halt GPIB transfers if the 7210 asserts its interrupt line
	 *	- Carry cycle is enabled, if EOI to be sent
	 *	- WORD (16 bit) transfers
	 */
		if (wrtop->io_ccfunc) {
			GPIBout(CCRG, wrtop->io_ccfunc);
			GPIBout(CFG, C_TLCH | C_CCEN | C_TMOE | C_T_B | C_B16);
		}
		else	GPIBout(CFG, C_TLCH | C_TMOE | C_T_B | C_B16);

		GPIBout(CNTL, -cnt);
		GPIBout(CNTH, -cnt >> 8);
		DBGprint(DBG_BRANCH, ("start DMA  "));
		ibcnt = cnt - osDoDMA(wrtop);
	}
	if (GPIBin(ISR1) & HR_ERR) {
		DBGprint(DBG_BRANCH, ("no listeners  "));
		ibsta |= ERR;
		iberr = ENOL;
	}
	else if (!(ibsta & ERR) && !noTimo) {
		DBGprint(DBG_BRANCH, ("timeout  "));
		ibsta |= (ERR | TIMO);
		iberr = EABO;
	}
	DBGout();
}


#else


/*
 *  BDWRT (PIO)
 *  This function performs a single Programmed I/O write operation.
 */
IBLCL void bdPIOwrt(ibio_op_t *wrtop)
{ 
	faddr_t		buf;
	unsigned	cnt;
	uint8		s1, s2;		/* software copies of HW status regs... */
	int		cfgbits;
	uint8		lsb;		/* unsigned residual LSB */
	char		msb;		/* signed residual MSB */

	DBGin("bdwrt");
	buf = wrtop->io_vbuf;
	cnt = wrtop->io_cnt;
	DBGprint(DBG_DATA, ("buf=0x%x cnt=%d  ", buf, cnt));

	GPIBout(IMR1, 0);
	GPIBout(IMR2, 0);		/* clear any previously arrived bits */

	s2 = GPIBin(ISR2);		/* clear the status registers... */
	s1 = GPIBin(ISR1);
	DBGprint(DBG_DATA, ("ISR1=0x%x ISR2=0x%x  ", s1, s2));

	DBGprint(DBG_BRANCH, ("reset FIFO, configure TURBO  "));
	GPIBout(CMDR, RSTFIFO);

	DBGprint(DBG_BRANCH, ("set-up EOT and EOS  "));
/*
 *	Set EOT and EOS modes...
 */
	if (!(wrtop->io_flags & IO_LAST) || (pgmstat & PS_NOEOI))
		cfgbits = C_TLCH | C_T_B;
					/* ...no default EOI w/last byte */
	else {
		cfgbits = C_TLCH | C_T_B | C_CCEN;
		GPIBout(CCRG, AUX_SEOI);
					/* ...send EOI on carry cycle */
	}
	GPIBout(AUXMR, auxrabits);	/* send EOI w/EOS if requested */
/*
 *	The configuration register in the gate-array is configured for
 *	- Halt GPIB transfers if the 7210 asserts its interrupt line
 *	- Carry cycle is enabled
 *	- BYTE (8 bit) transfers
 */
	GPIBout(CNTL, -cnt);
	GPIBout(CNTH, -cnt >> 8);
	GPIBout(CFG,  cfgbits);
	GPIBout(CMDR, GO);
	GPIBout(IMR1, HR_ERRIE);	/* set IMR1 before IMR2 on write */
	GPIBout(IMR2, HR_DMAO);

	DBGprint(DBG_BRANCH, ("begin PIO loop  "));
	while (ibcnt < cnt) {
		DBGprint(DBG_DATA, ("ISR3=0x%x  ", GPIBin(ISR3)));
		while (!(GPIBin(ISR3) & HR_NFF)) {
			if ((GPIBin(ISR3) & (HR_TLCI | HR_DONE)) || TimedOut())
			/*
			 *	NAT4882: If HR_TLCI is set, assume HR_ERR is
			 *	set in ISR1 because of a "no listeners" error.
			 */
				goto wrtdone;

			WaitingFor(HR_NFF | HR_TLCI | HR_DONE);
		}
		GPIBout(FIFOB, buf[ibcnt++]);
	}
/*
 *	If end of count then wait for ENOL, done, or timeout...
 */
	while (!(GPIBin(ISR3) & (HR_TLCI | HR_DONE)) && NotTimedOut()) {
		DBGprint(DBG_DATA, ("ISR3=0x%x  ", GPIBin(ISR3)));
		WaitingFor(HR_TLCI | HR_DONE);
	}
wrtdone:
	DBGprint(DBG_BRANCH, ("done  "));
	GPIBout(CMDR, STOP);
	GPIBout(IMR1, 0);		/* clear ERRIE if set */

	if (GPIBin(ISR1) & HR_ERR) {
		DBGprint(DBG_BRANCH, ("no listeners  "));
		ibsta |= ERR;
		iberr = ENOL;
	}
	else if (!noTimo) {
		DBGprint(DBG_BRANCH, ("timeout  "));
		ibsta |= (ERR | TIMO);
		iberr = EABO;
	}
	lsb = GPIBin(CNTL);
	msb = GPIBin(CNTH);
	ibcnt = cnt + (((int) lsb) | (((int) msb) << 8));
	DBGout();
}


#endif
