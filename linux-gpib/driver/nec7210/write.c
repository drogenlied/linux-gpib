
#include <board.h>



/*
 *  BDWRT (DMA)
 *  This function performs a single DMA write operation.
 */

#if DMAOP

IBLCL void bdDMAwrt(ibio_op_t *wrtop)
{

	faddr_t		buf;
	unsigned	cnt;

	int bytes=0;

	DBGin("bdwrt(dma)");

	buf = wrtop->io_vbuf;
	cnt = wrtop->io_cnt;

	DBGprint(DBG_DATA, ("buf=0x%p cnt=%d  ", buf, cnt));
	GPIBout(AUXMR, auxrabits);	/* send EOI w/EOS if requested */

        if( cnt == 1 ) { /* use PIO transfer */

	  DBGprint(DBG_BRANCH, ("begin PIO loop  "));
	  while (ibcnt < cnt) {
	    GPIBout(CDOR, buf[ibcnt]); 
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
	  GPIBout(AUXMR, AUX_SEOI);
	}
	GPIBout(CDOR, bdGetEOS() );
	bdWaitOut();

	DBGprint(DBG_BRANCH, ("done  "));

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

	ibcnt = bytes;

	DBGout();
}

#endif



/*
 *  BDWRT (PIO)
 *  This function performs a single Programmed I/O write operation.
 */
IBLCL void bdPIOwrt( ibio_op_t *wrtop)
{ 
	faddr_t		buf;
	unsigned	cnt;
	extern int eosmodes;
	int bytes=0;

	DBGin("bdwrt");

	buf = wrtop->io_vbuf;
	cnt = wrtop->io_cnt;

	DBGprint(DBG_DATA, ("buf=0x%p cnt=%d  ", buf, cnt));

	GPIBout(AUXMR, auxrabits);	/* send EOI w/EOS if requested */

	DBGprint(DBG_BRANCH, ("begin PIO loop  "));

	cnt-- ; /* save the last byte for sending EOI */

	while (ibcnt < cnt)
	{
		// XXX check for failure
		wait_event_interruptible(nec7210_write_wait, test_bit(0, &write_in_progress) == 0);
		set_bit(0, &write_in_progress);
		GPIBout(CDOR, buf[ibcnt]);
		bytes++;
		ibcnt++;
		if( TimedOut() ) break;
	}

	DBGprint(DBG_BRANCH, ("send EOI  "));
        /*send EOI */

	if( eosmodes & XEOS )
	{
		DBGprint(DBG_BRANCH, ("send EOS with EOI  "));
		// XXX check for failure
		wait_event_interruptible(nec7210_write_wait, test_bit(0, &write_in_progress) == 0);
		set_bit(0, &write_in_progress);
		GPIBout(CDOR, buf[ibcnt]);
		bytes++; ibcnt++;
		wait_event_interruptible(nec7210_write_wait, test_bit(0, &write_in_progress) == 0);
		set_bit(0, &write_in_progress);
		bdSendAuxCmd(AUX_SEOI);
		GPIBout(CDOR, bdGetEOS() );
	} else {
		DBGprint(DBG_BRANCH, ("send EOI with last byte "));
		wait_event_interruptible(nec7210_write_wait, test_bit(0, &write_in_progress) == 0);
		set_bit(0, &write_in_progress);
		bdSendAuxCmd(AUX_SEOI);
		GPIBout(CDOR, buf[ibcnt]);
		bytes++; ibcnt++;
	}
	wait_event_interruptible(nec7210_write_wait, test_bit(0, &write_in_progress) == 0);

	DBGprint(DBG_BRANCH, ("done  "));

/*
	if (GPIBin(ISR1) & HR_ERR) {
		DBGprint(DBG_BRANCH, ("no listeners  "));
		ibsta |= ERR;
		iberr = ENOL;
	}else
*/
	if (!noTimo) {
		DBGprint(DBG_BRANCH, ("timeout  "));
		ibsta |= (ERR | TIMO);
		iberr = EABO;
	}

	ibcnt = bytes;
	DBGout();
}













