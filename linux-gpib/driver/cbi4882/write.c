
#include "board.h"
#include <asm/io.h>

/*
 *  BDWRT (PIO)
 *  This function performs a single Programmed I/O write operation.
 */
IBLCL void bdwrt(ibio_op_t *wrtop)
{
	faddr_t		buf;
	unsigned	cnt;
	uint8_t		s1, s2;		/* software copies of HW status regs... */
	int		cfgbits;
	uint8_t		lsb;		/* unsigned residual LSB */
	char		msb;		/* signed residual MSB */
	int             chunk;          /* number of databytes to write in a chunk */
	int             odd;
	uint8_t           hs;
	extern int eosmodes;
	int bytes=0;

	DBGin("bdwrt");

	buf = wrtop->io_vbuf;
	cnt = wrtop->io_cnt;

	DBGprint(DBG_DATA, ("buf=0x%p cnt=%d  ", buf, cnt));

	GPIBout(IMR1, 0);
	GPIBout(IMR2, 0);		/* clear any previously arrived bits */

	s2 = GPIBin(ISR2);		/* clear the status registers... */
	s1 = GPIBin(ISR1);

	DBGprint(DBG_DATA, ("ISR1=0x%x ISR2=0x%x  ", s1, s2));

	GPIBout(AUXMR, auxrabits);	/* send EOI w/EOS if requested */

        cnt-- ; /* save the last byte for sending EOI */
#if ALPHA_TEST
	if( board_type == CBI_ISA_GPIB ) {

	  DBGprint(DBG_BRANCH, ("begin Polled FIFO loop  "));
	  GPIBout( HS_MODE, HS_RX_ENABLE | HS_TX_ENABLE | CurHSMode ); /*reset board*/
	  GPIBout( HS_MODE, CurHSMode ); /*reset board*/

	  GPIBout( HS_MODE, HS_CLR_HF_INT | HS_CLR_EOI_INT | HS_CLR_SRQ_INT  | CurHSMode );
	  GPIBout( HS_MODE, CurHSMode ); /* clear interrupt bits */

	  CurHSMode |= HS_TX_ENABLE;
	  GPIBout( HS_MODE, CurHSMode );
	  CurHSMode |= HS_HF_INT_EN;
	  GPIBout( HS_MODE, CurHSMode );
	  
	  GPIBout( IMR2, HR_DMAO );  /* enable nec7210 DMA out */

	  if( cnt > FIFO_SIZE ) { /* remaining bytes */
	    chunk = FIFO_SIZE/2;
	  } else {
	    chunk = cnt/2;
	  }
#error voelliger scheiss
printk("WRITING CHUNK: %d\n",chunk);
	  /* fill up the fifo */
	  outsw( &IB->CDOR , &(buf[ibcnt]), chunk );
	  ibcnt += chunk;
	  GPIBout( HS_MODE, HS_CLR_HF_INT | HS_CLR_EMPTY_INT | CurHSMode );
	  GPIBout( HS_MODE , CurHSMode );
	  while(! (hs = GPIBin(hs_status)) & HS_TX_MSB_EMPTY 
		&& ! hs & HS_HALF_FULL && !TimedOut() );
	  if( hs & HS_TX_MSB_EMPTY )
	    goto wrt1done;

	  while( cnt-ibcnt > FIFO_SIZE ) {
	    outsw( &IB->CDOR , &(buf[ibcnt]), chunk );
	    ibcnt += chunk;
	    GPIBout( HS_MODE, HS_CLR_HF_INT | HS_CLR_EMPTY_INT | CurHSMode );
	    GPIBout( HS_MODE , CurHSMode );
	    while(! (hs = GPIBin(hs_status)) & HS_TX_MSB_EMPTY 
                     && ! hs & HS_HALF_FULL && !TimedOut() );
	    if( hs & HS_TX_MSB_EMPTY )
	    goto wrt1done;
	  }
wrt1done:
	  CurHSMode &= ~(HS_TX_ENABLE+HS_HF_INT_EN);
	  GPIBout(HS_MODE, CurHSMode );
	  GPIBout(IMR2,0 );
printk("REMAINDER: %d\n",cnt-ibcnt);
	  while (ibcnt < cnt) {  /* write remaining bytes */
	    GPIBout(CDOR, buf[ibcnt]); 
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
	    GPIBout(CDOR, buf[ibcnt]); 
	    bytes++;
	    /*printk("out=%c\n",buf[ibcnt]);*/
	    ibcnt++;
	    bdWaitOut();
	    if( TimedOut() ) break;
	  }
#if ALPHA_TEST 
	}
#endif

	DBGprint(DBG_BRANCH, ("send EOI  "));
        /*send EOI */


	  DBGprint(DBG_BRANCH, ("send EOI with last byte "));
	  bdSendAuxCmd(AUX_SEOI);
	  GPIBout(CDOR, buf[ibcnt]);
	  bytes++; ibcnt++;
	bdWaitOut();


	DBGprint(DBG_BRANCH, ("done  "));
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

	ibcnt = bytes;
	DBGout();
}








