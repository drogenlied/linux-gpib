
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

	GPIBout(imr1, 0);
	GPIBout(imr2, 0);		/* clear any previously arrived bits */

	s2 = GPIBin(isr2);		/* clear the status registers... */
	s1 = GPIBin(isr1);
	DBGprint(DBG_DATA, ("isr1=0x%x isr2=0x%x  ", s1, s2));

	DBGprint(DBG_BRANCH, ("reset FIFO, configure TURBO  "));
	GPIBout(cmdr, RSTFIFO);

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

	GPIBout(auxmr, auxrabits);	/* send EOI w/EOS if requested */

	if (cnt == 1) {			/* one byte always PIO... */
	/*
	 *	The configuration register in the gate-array is configured for
	 *	- Halt GPIB transfers if the 7210 asserts its interrupt line
	 *	- Carry cycle is enabled
	 *	- BYTE (8 bit) transfers
	 */
		if (wrtop->io_ccfunc) {
			GPIBout(ccrg, wrtop->io_ccfunc);
			GPIBout(cfg, (C_TLCH | C_T_B | C_CCEN));
		}
		else	GPIBout(cfg, (C_TLCH | C_T_B));

		GPIBout(cntl, -cnt);
		GPIBout(cnth, -cnt >> 8);
		GPIBout(cmdr, GO);
		GPIBout(imr1, HR_ERRIE);/* set imr1 before imr2 on write */
		GPIBout(imr2, HR_DMAO);
	
		DBGprint(DBG_BRANCH, ("send out single PIO byte  "));
		DBGprint(DBG_DATA, ("isr3=0x%x  ", GPIBin(isr3)));
	/*
	 *	After "RSTFIFO" above, FIFO should be empty, so assume
	 *	HR_NFF is set.  After writing out the byte, wait for
	 *	ENOL, done, or timeout.
	 */
		GPIBout(fifo.f8.b, buf[ibcnt++]);
		while (!(GPIBin(isr3) & (HR_TLCI | HR_DONE)) && NotTimedOut()) {
		/*
		 *	NAT4882: If HR_TLCI is set, assume HR_ERR is
		 *	set in isr1 because of a "no listeners" error.
		 */
			DBGprint(DBG_DATA, ("isr3=0x%x  ", GPIBin(isr3)));
			WaitingFor(HR_TLCI | HR_DONE);
		}
		DBGprint(DBG_BRANCH, ("done  "));
		GPIBout(cmdr, STOP);
		GPIBout(imr1, 0);	/* clear ERRIE if set */
		if (GPIBin(cntl))
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
			GPIBout(ccrg, wrtop->io_ccfunc);
			GPIBout(cfg, C_TLCH | C_CCEN | C_TMOE | C_T_B | C_B16);
		}
		else	GPIBout(cfg, C_TLCH | C_TMOE | C_T_B | C_B16);

		GPIBout(cntl, -cnt);
		GPIBout(cnth, -cnt >> 8);
		DBGprint(DBG_BRANCH, ("start DMA  "));
		ibcnt = cnt - osDoDMA(wrtop);
	}
	if (GPIBin(isr1) & HR_ERR) {
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

	GPIBout(imr1, 0);
	GPIBout(imr2, 0);		/* clear any previously arrived bits */

	s2 = GPIBin(isr2);		/* clear the status registers... */
	s1 = GPIBin(isr1);
	DBGprint(DBG_DATA, ("isr1=0x%x isr2=0x%x  ", s1, s2));

	DBGprint(DBG_BRANCH, ("reset FIFO, configure TURBO  "));
	GPIBout(cmdr, RSTFIFO);

	DBGprint(DBG_BRANCH, ("set-up EOT and EOS  "));
/*
 *	Set EOT and EOS modes...
 */
	if (!(wrtop->io_flags & IO_LAST) || (pgmstat & PS_NOEOI))
		cfgbits = C_TLCH | C_T_B;
					/* ...no default EOI w/last byte */
	else {
		cfgbits = C_TLCH | C_T_B | C_CCEN;
		GPIBout(ccrg, AUX_SEOI);
					/* ...send EOI on carry cycle */
	}
	GPIBout(auxmr, auxrabits);	/* send EOI w/EOS if requested */
/*
 *	The configuration register in the gate-array is configured for
 *	- Halt GPIB transfers if the 7210 asserts its interrupt line
 *	- Carry cycle is enabled
 *	- BYTE (8 bit) transfers
 */
	GPIBout(cntl, -cnt);
	GPIBout(cnth, -cnt >> 8);
	GPIBout(cfg,  cfgbits);
	GPIBout(cmdr, GO);
	GPIBout(imr1, HR_ERRIE);	/* set imr1 before imr2 on write */
	GPIBout(imr2, HR_DMAO);

	DBGprint(DBG_BRANCH, ("begin PIO loop  "));
	while (ibcnt < cnt) {
		DBGprint(DBG_DATA, ("isr3=0x%x  ", GPIBin(isr3)));
		while (!(GPIBin(isr3) & HR_NFF)) {
			if ((GPIBin(isr3) & (HR_TLCI | HR_DONE)) || TimedOut())
			/*
			 *	NAT4882: If HR_TLCI is set, assume HR_ERR is
			 *	set in isr1 because of a "no listeners" error.
			 */
				goto wrtdone;

			WaitingFor(HR_NFF | HR_TLCI | HR_DONE);
		}
		GPIBout(fifo.f8.b, buf[ibcnt++]);
	}
/*
 *	If end of count then wait for ENOL, done, or timeout...
 */
	while (!(GPIBin(isr3) & (HR_TLCI | HR_DONE)) && NotTimedOut()) {
		DBGprint(DBG_DATA, ("isr3=0x%x  ", GPIBin(isr3)));
		WaitingFor(HR_TLCI | HR_DONE);
	}
wrtdone:
	DBGprint(DBG_BRANCH, ("done  "));
	GPIBout(cmdr, STOP);
	GPIBout(imr1, 0);		/* clear ERRIE if set */

	if (GPIBin(isr1) & HR_ERR) {
		DBGprint(DBG_BRANCH, ("no listeners  "));
		ibsta |= ERR;
		iberr = ENOL;
	}
	else if (!noTimo) {
		DBGprint(DBG_BRANCH, ("timeout  "));
		ibsta |= (ERR | TIMO);
		iberr = EABO;
	}
	lsb = GPIBin(cntl);
	msb = GPIBin(cnth);
	ibcnt = cnt + (((int) lsb) | (((int) msb) << 8));
	DBGout();
}


#endif
