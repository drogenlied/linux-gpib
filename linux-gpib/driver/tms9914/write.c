#include "board.h"


/*
 *  BDWRT (PIO)
 *  This function performs a single Programmed I/O write operation.
 */
IBLCL void bdwrt(ibio_op_t *wrtop)
{ 
	faddr_t		buf;
	unsigned	cnt;
	int		s1, s2;		/* software copies of HW status regs... */
	int		cfgbits;
	uint8_t		lsb;		/* unsigned residual LSB */
	char		msb;		/* signed residual MSB */
extern  int eosmodes;
        int bytes=0;

	DBGin("bdwrt");

	buf = wrtop->io_vbuf;
	cnt = wrtop->io_cnt;

	DBGprint(DBG_DATA, ("buf=0x%p cnt=%d  ", buf, cnt));

	GPIBout(IMR0, 0);
	GPIBout(IMR1, 0);		/* clear any previously arrived bits */

	s2 = GPIBin(ISR1);		/* clear the status registers... */
	s1 = GPIBin(ISR0);

	DBGprint(DBG_DATA, ("ISR1=0x%x ISR2=0x%x  ", s1, s2));

	DBGprint(DBG_BRANCH, ("begin PIO loop  "));

        cnt-- ; /* save the last byte for sending EOI */


	while (ibcnt < cnt) {
		GPIBout(CDOR, buf[ibcnt]); 
                bytes++;
                /*printk("out=%c\n",buf[ibcnt]);*/
                ibcnt++;
		bdWaitOut();
		if( TimedOut() ) break;
	}
wrtdone:

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













