
#include <board.h>
#include <asm/io.h>


/*
 *  BDWRT (DMA)
 *  This function performs a single DMA write operation.
 */

#if DMAOP

IBLCL void bdDMAwrt(ibio_op_t *wrtop)
{ 

	faddr_t		buf;
	unsigned	cnt;
	uint8		s1, s2;		/* software copies of HW status regs */

        int bytes=0;

	DBGin("bdwrt(dma)");

	buf = wrtop->io_vbuf;
	cnt = wrtop->io_cnt;

	DBGprint(DBG_DATA, ("buf=0x%x cnt=%d  ", buf, cnt));

	GPIBout(imr1, 0);
	GPIBout(imr2, 0);		/* clear any previously arrived bits */

	s2 = GPIBin(isr2);		/* clear the status registers... */
	s1 = GPIBin(isr1);

	DBGprint(DBG_DATA, ("isr1=0x%x isr2=0x%x  ", s1, s2));

	GPIBout(auxmr, auxrabits);	/* send EOI w/EOS if requested */

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
}

#endif



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
	int             chunk;          /* number of databytes to write in a chunk */
	int             odd;
	uint8           hs;
extern int eosmodes;
        int bytes=0;

	DBGin("bdwrt");

	buf = wrtop->io_vbuf;
	cnt = wrtop->io_cnt;

	DBGprint(DBG_DATA, ("buf=0x%p cnt=%d  ", buf, cnt));

	GPIBout(imr1, 0);
	GPIBout(imr2, 0);		/* clear any previously arrived bits */

	s2 = GPIBin(isr2);		/* clear the status registers... */
	s1 = GPIBin(isr1);

	DBGprint(DBG_DATA, ("isr1=0x%x isr2=0x%x  ", s1, s2));

	GPIBout(auxmr, auxrabits);	/* send EOI w/EOS if requested */

        cnt-- ; /* save the last byte for sending EOI */
#if ALPHA_TEST
	if( board_type == CBI_ISA_GPIB ) {

	  DBGprint(DBG_BRANCH, ("begin Polled FIFO loop  "));
	  GPIBout( hs_mode, HS_RX_ENABLE | HS_TX_ENABLE | CurHSMode ); /*reset board*/
	  GPIBout( hs_mode, CurHSMode ); /*reset board*/
	  
	  GPIBout( hs_mode, HS_CLR_HF_INT | HS_CLR_EOI_INT | HS_CLR_SRQ_INT  | CurHSMode );
	  GPIBout( hs_mode, CurHSMode ); /* clear interrupt bits */

	  CurHSMode |= HS_TX_ENABLE;
	  GPIBout( hs_mode, CurHSMode );
	  CurHSMode |= HS_HF_INT_EN;
	  GPIBout( hs_mode, CurHSMode );
	  
	  GPIBout( imr2, HR_DMAO );  /* enable nec7210 DMA out */

	  if( cnt > FIFO_SIZE ) { /* remaining bytes */
	    chunk = FIFO_SIZE/2;
	  } else {
	    chunk = cnt/2;
	  }
#error voelliger scheiss
printk("WRITING CHUNK: %d\n",chunk);
	  /* fill up the fifo */
	  outsw( &IB->cdor , &(buf[ibcnt]), chunk );
	  ibcnt += chunk;
	  GPIBout( hs_mode, HS_CLR_HF_INT | HS_CLR_EMPTY_INT | CurHSMode );
	  GPIBout( hs_mode , CurHSMode );
	  while(! (hs = GPIBin(hs_status)) & HS_TX_MSB_EMPTY 
		&& ! hs & HS_HALF_FULL && !TimedOut() );
	  if( hs & HS_TX_MSB_EMPTY )
	    goto wrt1done;

	  while( cnt-ibcnt > FIFO_SIZE ) {
	    outsw( &IB->cdor , &(buf[ibcnt]), chunk );
	    ibcnt += chunk;
	    GPIBout( hs_mode, HS_CLR_HF_INT | HS_CLR_EMPTY_INT | CurHSMode );
	    GPIBout( hs_mode , CurHSMode );
	    while(! (hs = GPIBin(hs_status)) & HS_TX_MSB_EMPTY 
                     && ! hs & HS_HALF_FULL && !TimedOut() );
	    if( hs & HS_TX_MSB_EMPTY )
	    goto wrt1done;
	  }
wrt1done:
	  CurHSMode &= ~(HS_TX_ENABLE+HS_HF_INT_EN);
	  GPIBout(hs_mode, CurHSMode );
	  GPIBout(imr2,0 );
printk("REMAINDER: %d\n",cnt-ibcnt);
	  while (ibcnt < cnt) {  /* write remaining bytes */
	    GPIBout(cdor, buf[ibcnt]); 
	    bytes++;
	    /*printk("out=%c\n",buf[ibcnt]);*/
	    ibcnt++;
	    bdWaitOut();
	    if( TimedOut() ) break;
	  }
	  



	  /*error look here*/
	} else { /* isa gpib */
#endif
	  DBGprint(DBG_BRANCH, ("begin PIO loop  "));

	  while (ibcnt < cnt) {
	    GPIBout(cdor, buf[ibcnt]); 
	    bytes++;
	    /*printk("out=%c\n",buf[ibcnt]);*/
	    ibcnt++;
	    bdWaitOut();
	    if( TimedOut() ) break;
	  }
#if ALPHA_TEST 
	}
#endif

wrtdone:
	DBGprint(DBG_BRANCH, ("send EOI  "));
        /*send EOI */


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








