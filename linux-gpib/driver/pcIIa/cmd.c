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
	uint8		s1, s2;		/* software copies of HW status regs */
	int             bytes=0;

	DBGin("bdcmd");
	buf = cmdop->io_vbuf;
	cnt = cmdop->io_cnt;
	DBGprint(DBG_DATA, ("buf=0x%x cnt=%d  ", buf, cnt));

	GPIBout(imr1, 0);
	GPIBout(imr2, 0);		/* clear any previously arrived bits */

	s2 = GPIBin(isr2);		/* clear the status registers... */
	s1 = GPIBin(isr1);
	DBGprint(DBG_DATA, ("isr1=0x%x isr2=0x%x  ", s1, s2));

	DBGprint(DBG_BRANCH, ("begin PIO loop  "));
	while (ibcnt < cnt) {
		GPIBout(cdor, buf[ibcnt]); 
		bdWaitOut();
                ibcnt++; bytes ++;
	}
	DBGprint(DBG_BRANCH, ("wait for DONE  "));

cmddone:
	GPIBout(auxmr, AUX_GTS);	/* go to standby */

	if (!noTimo) {
		DBGprint(DBG_BRANCH, ("timeout  "));
		ibsta |= (ERR | TIMO);
		iberr = EABO;
	}
	DBGprint(DBG_BRANCH, ("done  "));
	GPIBout(imr2, 0);		/* clear COIE if set */
	ibcnt = bytes;
	DBGout();
}









