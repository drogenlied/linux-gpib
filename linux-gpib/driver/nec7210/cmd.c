#include <board.h>


/*
 *  BDCMD
 *  This function performs a single Programmed I/O command operation.
 *  Note that ATN must already be asserted when this function is called.
 */
IBLCL void bdcmd(ibio_op_t *cmdop)
{
	faddr_t		buf;
	unsigned	cnt;
	int             bytes=0;

	DBGin("bdcmd");
	buf = cmdop->io_vbuf;
	cnt = cmdop->io_cnt;
	DBGprint(DBG_DATA, ("buf=0x%p cnt=%d  ", buf, cnt));
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
	DBGprint(DBG_BRANCH, ("done  "));
	ibcnt = bytes;
	DBGout();
}









