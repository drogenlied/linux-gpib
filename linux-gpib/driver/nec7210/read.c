
#include "board.h"

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
	unsigned long flags;

	DBGin("bdread(dma)");
	buf = rdop->io_vbuf;
	cnt = rdop->io_cnt;
	DBGprint(DBG_DATA, ("bdread: buf=0x%p cnt=%d  ", buf, cnt));
	if (pgmstat & PS_HELD) {
		DBGprint(DBG_BRANCH, ("finish handshake  "));
		GPIBout(AUXMR, auxrabits | HR_HLDA);
		GPIBout(AUXMR, AUX_FH);	/* set HLDA in AUXRA to ensure FH works */
		pgmstat &= ~PS_HELD;
	}else
	{
		spinlock_irqsave(&status_lock, flags);
		if ((s1 & HR_DI) && (s1 & HR_END))
		{
			DBGprint(DBG_BRANCH, ("one-byte read with END  "));
			GPIBout(AUXMR, auxrabits | HR_HLDA);
			pgmstat |= PS_HELD;
			buf[0] = GPIBin(DIR);
			ibsta |= END;
			ibcnt = 1;
			DBGout();
			return;
		}
	}
	DBGprint(DBG_BRANCH, ("set-up EOS modes  "));
/*
 *	Set EOS modes, holdoff on END, and holdoff on all carry cycle...
 */
	GPIBout(AUXMR, auxrabits | HR_HLDE ); /*| HR_REOS );*/
                                              /* no longer hardwired */


	if ( cnt == 1 ){   /* Use PIO Transfer  */

	  /* if first data byte availiable read once */
	  /*
	   * This is a relatively clean solution for the EOS detection problem
	   *
	   */

	  if( s1 & HR_DI )
	    buf[ibcnt++] = GPIBin(DIR);


	  DBGprint(DBG_BRANCH, ("begin PIO loop  "));
	  while (ibcnt < cnt-1) {

	    if( bdWaitIn() < 0 ) {         
              /* end is set with EOS byte so wait first */
	      ibsta |= END;
	      break;
	    }
	    buf[ibcnt++] = GPIBin(DIR);
	    /*printk("buf[%d]='%c'",ibcnt-1,buf[ibcnt-1]);*/
	  }
	  GPIBout(AUXMR, auxrabits | HR_HLDA);
	  buf[ibcnt++]=GPIBin(DIR);           /* read last byte on end */
//	  s1 = GPIBin(ISR1);

	} else {        /* Use DMA */
	  DBGprint(DBG_BRANCH, ("start DMA cycle  "));

	  rdop->io_cnt = rdop->io_cnt - 1;
	  cnt = rdop->io_cnt;

	  ibcnt = cnt - osDoDMA(rdop);

//	  s1 = GPIBin(ISR1);
	  if ( s1 & HR_ENDIE ){
	    ibsta |= END;
	  } else {
	    buf[ibcnt++]=GPIBin(DIR);           /* read last byte on end */
	  }
        }

	if ((ibsta & END) || (ibcnt == cnt)) {
		pgmstat |= PS_HELD;
        }

	if (!noTimo) {
		DBGprint(DBG_BRANCH, ("timeout  "));
		ibsta |= (ERR | TIMO);
		iberr = EABO;
	}


	DBGout();
}



#endif

/*
 *  BDREAD (PIO)
 *  This function performs a single Programmed I/O read operation.
 *  Note that the hand-shake is held off at the end of every read.
 */
IBLCL void bdPIOread(ibio_op_t *rdop)
{
	faddr_t		buf;
	unsigned	cnt;
	gpib_char_t data;
	int ret;
	unsigned long flags;

	DBGin("bdread");

	buf = rdop->io_vbuf;
	cnt = rdop->io_cnt;
	DBGprint(DBG_DATA, ("bdread: buf=0x%p cnt=%d  ", buf, cnt));
	if (pgmstat & PS_HELD) {
		DBGprint(DBG_BRANCH, ("finish handshake  "));
		GPIBout(AUXMR, auxrabits | HR_HLDA);
		GPIBout(AUXMR, AUX_FH);	/* set HLDA in AUXRA to ensure FH works */
		pgmstat &= ~PS_HELD;
	}
	DBGprint(DBG_BRANCH, ("set-up EOS modes  "));
/*
 *	Set EOS modes, holdoff on END, and holdoff on all carry cycle...
 */
	GPIBout(AUXMR, auxrabits | HR_HLDE );

	/* if first data byte availiable read once */
	/*
	* This is a relatively clean solution for the EOS detection problem
	*
	*/

	DBGprint(DBG_BRANCH, ("begin PIO loop  "));
	while (ibcnt < cnt )
	{
		spin_lock_irqsave(&read_buffer->lock, flags);
		ret = gpib_buffer_get(read_buffer, &data);
		spin_unlock_irqrestore(&read_buffer->lock, flags);
		if(ret < 0)
		{
			printk("gpib:no data\n");
			if(wait_event_interruptible(nec7210_read_wait, read_buffer->size > 0))
			{
				printk("wait failed\n");
				// XXX
			};
			continue;
		}
		buf[ibcnt++] = data.value;
		if(data.end)
		{
			ibsta |= END;
			break;
		}
	}
	GPIBout(AUXMR, auxrabits | HR_HLDA );
	pgmstat |= PS_HELD;

	DBGprint(DBG_BRANCH, ("done  "));

	if (!noTimo)
	{
		DBGprint(DBG_BRANCH, ("timeout  "));
		ibsta |= (ERR | TIMO);
		iberr = EABO;
	}
	DBGout();
}








