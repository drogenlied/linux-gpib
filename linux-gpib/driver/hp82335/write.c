
#include <board.h>



/*
 *  BDWRT (DMA)
 *  This function performs a single DMA write operation.
 */

#if DMAOP

IBLCL bdDMAwrt(ibio_op_t *wrtop)
{ 

#if 0
	faddr_t		buf;
	unsigned	cnt;
	uint8		s1, s2;		/* software copies of HW status regs */

        int bytes=0;

	DBGin("bdwrt(dma)");

	buf = wrtop->io_vbuf;
	cnt = wrtop->io_cnt;

	DBGprint(DBG_DATA, ("buf=0x%x cnt=%d  ", buf, cnt));

	GPIBout(imr0, 0);
	GPIBout(imr1, 0);		/* clear any previously arrived bits */

	s2 = GPIBin(isr1);		/* clear the status registers... */
	s1 = GPIBin(isr0);

	DBGprint(DBG_DATA, ("isr1=0x%x isr2=0x%x  ", s1, s2));

	/* GPIBout(auxmr, auxrabits);*/	/* send EOI w/EOS if requested */

        if( cnt == 1 ) { /* use PIO transfer */

	  DBGprint(DBG_BRANCH, ("begin PIO loop  "));
	  while (ibcnt < cnt) {
	    GPIBout(cdor, buf[ibcnt]); 
	    bytes++;
	    ibcnt++;
	    bdWaitOut();
	    if( TimedOut() ) break;
	  }

	} else { /* DMA */

	  DBGprint(DBG_BRANCH, ("start DMA cycle  "));
	  bytes = cnt - osDoDMA(wrtop);

	}

      wrtdone:

        if( !( pgmstat & PS_NOEOI) ) {
	  DBGprint(DBG_BRANCH, ("send EOI  "));
	  GPIBout(auxmr, AUX_SEOI);
	}
	GPIBout(cdor, bdGetEOS() );
	bdWaitOut();

	DBGprint(DBG_BRANCH, ("done  "));
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

	ibcnt = bytes;

	DBGout();
#endif
}

#endif



/*
 *  BDWRT (PIO)
 *  This function performs a single Programmed I/O write operation.
 */
IBLCL bdPIOwrt(ibio_op_t *wrtop)
{ 
	faddr_t		buf;
	unsigned	cnt;
	uint8		s1, s2;		/* software copies of HW status regs... */
	int		cfgbits;
	uint8		lsb;		/* unsigned residual LSB */
	char		msb;		/* signed residual MSB */
extern  int eosmodes;
        int bytes=0;

	DBGin("bdwrt");

	buf = wrtop->io_vbuf;
	cnt = wrtop->io_cnt;

	DBGprint(DBG_DATA, ("buf=0x%x cnt=%d  ", buf, cnt));

	GPIBout(imr0, 0);
	GPIBout(imr1, 0);		/* clear any previously arrived bits */

	s2 = GPIBin(isr1);		/* clear the status registers... */
	s1 = GPIBin(isr0);

	DBGprint(DBG_DATA, ("isr1=0x%x isr2=0x%x  ", s1, s2));
#if 0
	GPIBout(auxmr, auxrabits);	/* send EOI w/EOS if requested */
#endif


#define FIX_EOS_BUG 1 

	DBGprint(DBG_BRANCH, ("begin PIO loop  "));

#if FIX_EOS_BUG
        cnt-- ; /* save the last byte for sending EOI */
#endif


	while (ibcnt < cnt) {
		GPIBout(cdor, buf[ibcnt]); 
                bytes++;
                /*printk("out=%c\n",buf[ibcnt]);*/
                ibcnt++;
		bdWaitOut();
		if( TimedOut() ) break;
	}
wrtdone:

#if FIX_EOS_BUG
	if( eosmodes & XEOS ) {
          DBGprint(DBG_BRANCH, ("send EOS with EOI  "));
	  GPIBout(cdor, buf[ibcnt]);
	  bdWaitOut();
	  bytes++; ibcnt++;
	  bdSendAuxCmd(AUX_SEOI);
	  GPIBout(cdor, bdGetEOS() );
	} else {
	  DBGprint(DBG_BRANCH, ("send EOI with last byte "));
	  bdSendAuxCmd(AUX_SEOI);
	  GPIBout(cdor, buf[ibcnt]);
	  bytes++; ibcnt++;
	}
#else
	GPIBout(auxcr, AUX_SEOI);
	GPIBout(cdor, bdGetEOS() );
#endif

	bdWaitOut();


	DBGprint(DBG_BRANCH, ("done  "));
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

	ibcnt = bytes;
	DBGout();
}













