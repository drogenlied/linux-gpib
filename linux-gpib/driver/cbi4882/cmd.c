#include "board.h"


/*
 *  BDCMD
 *  This function performs a single Programmed I/O command operation.
 *  Note that ATN must already be asserted when this function is called.
 */
IBLCL void bdcmd(ibio_op_t *cmdop)
{
	faddr_t		buf;
	unsigned	cnt;
	uint8		s1, s2;		/* software copies of HW status regs */
	int             bytes=0;

	DBGin("bdcmd");
	buf = cmdop->io_vbuf;
	cnt = cmdop->io_cnt;
	DBGprint(DBG_DATA, ("buf=0x%p cnt=%d  ", buf, cnt));

	GPIBout(IMR1, 0);
	GPIBout(IMR2, 0);		/* clear any previously arrived bits */

	s2 = GPIBin(ISR2);		/* clear the status registers... */
	s1 = GPIBin(ISR1);
	DBGprint(DBG_DATA, ("ISR1=0x%x ISR2=0x%x  ", s1, s2));

        CurHSMode &= ~(HS_RX_ENABLE+HS_TX_ENABLE);
	/*CurHSMode |= HS_TX_ENABLE;*/

	GPIBout(HS_MODE, CurHSMode );


	DBGprint(DBG_BRANCH, ("begin PIO loop  "));
	while (ibcnt < cnt) {
		GPIBout(CDOR, buf[ibcnt]); 
		bdWaitOut();
                ibcnt++; bytes ++;
	}
	DBGprint(DBG_BRANCH, ("wait for DONE  "));

	GPIBout(AUXMR, AUX_GTS);	/* go to standby */

	if (!noTimo) {
		DBGprint(DBG_BRANCH, ("timeout  "));
		ibsta |= (ERR | TIMO);
		iberr = EABO;
	}
        DBGprint(DBG_DATA,("**brdstat=0x%x\n",GPIBin(ADSR)) );
	DBGprint(DBG_BRANCH,("done  ") );
	GPIBout(IMR2, 0);		/* clear COIE if set */
	ibcnt = bytes;
	DBGout();
}









