
#include <board.h>


/*
 *  BDREAD (DMA)
 *  This function performs a single DMA read operation.
 *  Note that the hand-shake is held off at the end of every read.
 */
#if DMAOP
#error DMA is not supported with this board
IBLCL void bdDMAread(ibio_op_t *rdop)
{

	faddr_t		buf;
	unsigned	cnt;
	uint8		s1, s2;		/* software copies of HW status regs */

	DBGin("bdread(dma)");
	buf = rdop->io_vbuf;
	cnt = rdop->io_cnt;
	DBGprint(DBG_DATA, ("bdread: buf=0x%x cnt=%d  ", buf, cnt));

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
	DBGprint(DBG_BRANCH, ("set-up EOS modes  "));
/*
 *	Set EOS modes, holdoff on END, and holdoff on all carry cycle...
 */
	GPIBout(auxmr, auxrabits | HR_HLDE ); /*| HR_REOS );*/
                                              /* no longer hardwired */


	if ( cnt == 1 ){   /* Use PIO Transfer  */

	  /* if first data byte availiable read once */
	  /*
	   * This is a relatively clean solution for the EOS detection problem
	   *
	   */

	  if( s1 & HR_DI )
	    buf[ibcnt++] = GPIBin(dir);


	  DBGprint(DBG_BRANCH, ("begin PIO loop  "));
	  while (ibcnt < cnt-1) {

	    if( bdWaitIn() < 0 ) {         
              /* end is set with EOS byte so wait first */
	      ibsta |= END;
	      break;
	    }
	    buf[ibcnt++] = GPIBin(dir);
	    /*printk("buf[%d]='%c'",ibcnt-1,buf[ibcnt-1]);*/
	  }
	  GPIBout(auxmr, auxrabits | HR_HLDA);
	  buf[ibcnt++]=GPIBin(dir);           /* read last byte on end */
	  s1 = GPIBin(isr1);
	  GPIBout(imr1, 0);

	} else {        /* Use DMA */
	  DBGprint(DBG_BRANCH, ("start DMA cycle  "));

	  rdop->io_cnt = rdop->io_cnt - 1;
	  cnt = rdop->io_cnt;

	  ibcnt = cnt - osDoDMA(rdop);

	  s1 = GPIBin(isr1);
	  if ( s1 & HR_ENDIE ){
	    ibsta |= END;
	  } else {
	    buf[ibcnt++]=GPIBin(dir);           /* read last byte on end */
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
	uint8		s1, s2;		/* software copies of HW status regs */
	int             isreg1 = 0;

	DBGin("bdread");

	buf = rdop->io_vbuf;
	cnt = rdop->io_cnt;
	DBGprint(DBG_DATA, ("bdread: buf=0x%p cnt=%d  ", buf, cnt));

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
	DBGprint(DBG_BRANCH, ("set-up EOS modes  "));
/*
 *	Set EOS modes, holdoff on END, and holdoff on all carry cycle...
 */
	GPIBout(auxmr, auxrabits | HR_HLDE ); /*| HR_REOS );*/
                                              /* no longer hardwired */

        /* if first data byte availiable read once */
        /*
         * This is a relatively clean solution for the EOS detection problem
         *
         */

        if( s1 & HR_DI )
		  buf[ibcnt++] = GPIBin(dir);


	DBGprint(DBG_BRANCH, ("begin PIO loop  "));
	/*@*/
#ifdef GREG_BUGFIX  /* warning this produces errors for me! */
	while (ibcnt < cnt - 1 ) { 

		if( (isreg1 = bdWaitIn()) == 0 ) {         /* end is set with EOS byte so wait first */
		  ibsta |= END;
		  break;
		}
		buf[ibcnt++] = GPIBin(dir);
                printk("buf[%d]='%c'",ibcnt-1,buf[ibcnt-1]);
	}
#else
	while (ibcnt < cnt ) {

	  isreg1 = bdWaitIn();
	  if ( isreg1 == 0 ) {
	    ibsta |= END;
	    break;
	  } else if ( isreg1 & HR_END ) {
	    ibsta |= END;
	    /*buf[ibcnt++] = GPIBin(dir);*/
	    break;
	  }
	  if ( ibcnt >= cnt-1 ) {
	    GPIBout(auxmr, auxrabits | HR_HLDA ); 
	  }
	  buf[ibcnt++] = GPIBin(dir);
	}

        if ( isreg1 & HR_END || ibcnt == cnt ) {
          pgmstat |= PS_HELD;
        }

        
#endif
        /*@*/
	GPIBout(auxmr, auxrabits | HR_HLDA );/* avoid last byte getting lost */
        buf[ibcnt++]=GPIBin(dir);           /* read last byte on end */

rddone:
	DBGprint(DBG_BRANCH, ("done  "));

	GPIBout(imr1, 0);		/* clear ENDIE if set */


	if (GPIBin(isr1) & HR_END)
		ibsta |= END;
#ifndef GREG_BUGFIX
	if ((ibsta & END) || (ibcnt == cnt))
#else
	if ((ibsta & END) || (ibcnt >= cnt))
#endif
		pgmstat |= PS_HELD;
	if (!noTimo) {
		DBGprint(DBG_BRANCH, ("timeout  "));
		ibsta |= (ERR | TIMO);
		iberr = EABO;
	}
	DBGout();
}








