#include <board.h>

int cmdstat=0x0;


#define TAS 1
#define LAS 2
#define FORCE_OUT 4

#define ADRMASK 0x1f /* lower 5 bits are device adress */


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
extern  int             myPAD;
        int i;

	DBGin("bdcmd");
	buf = cmdop->io_vbuf;
	cnt = cmdop->io_cnt;
	DBGprint(DBG_DATA, ("buf=0x%p cnt=%d  ", buf, cnt));


	GPIBout(imr0, 0);
	GPIBout(imr1, 0);		/* clear any previously arrived bits */

	s2 = GPIBin(isr1);		/* clear the status registers... */
	s1 = GPIBin(isr0);
	DBGprint(DBG_DATA, ("isr0=0x%x isr1=0x%x  ", s1, s2));



	DBGprint(DBG_BRANCH, ("begin PIO loop  "));
	while (ibcnt < cnt) {

          /* the 9914 has no full controller function so adressing itself to
           * talker or listener state must be done by TON or LON commands.
	   * This may affect the raw bus-control commands, its better to send
           * always an UNT or UNL as first message.
           *
	   */

	  /*DBGprint(DBG_DATA,("adsr(in)=0x%x",GPIBin(adsr)));*/

	  if( buf[ibcnt] == UNL || buf[ibcnt] == UNT ){
	    if( GPIBin(adsr) & HR_TA ){
	      DBGprint(DBG_BRANCH,
		       ("*** disable talker state "));
	      GPIBout(auxcr,AUX_TON ); /* enable talker */
	    } 
	    if( GPIBin(adsr) & HR_LA ){
	      DBGprint(DBG_BRANCH, 
		       ("*** disable listener state"));
	      GPIBout(auxcr,AUX_LON ); /* enable talker */
	    }
	  } 
	  if( buf[ibcnt] == (myPAD|TAD) ){ 
	      DBGprint(DBG_BRANCH, 
		       ("*** enable talker state "));
	      GPIBout(auxcr,AUX_TON | AUX_CS); /* enable talker */
	  }  
	  if (buf[ibcnt] == (myPAD|LAD)) {
		DBGprint(DBG_BRANCH, 
			 ("*** enable listener state "));
		GPIBout(auxcr,AUX_LON | AUX_CS); /* enable listener */
	  } 

          /*DBGprint(DBG_DATA,("adsr=0x%x",GPIBin(adsr)));*/

	  { /* put other commands on the bus */
                DBGprint(DBG_DATA,("cmd=0x%x",buf[ibcnt]));
		GPIBout(cdor, buf[ibcnt]); 
		bdWaitOut();
	  }


          ibcnt++; bytes ++;
	}
	DBGprint(DBG_BRANCH, ("wait for DONE  "));

cmddone:
        DBGprint(DBG_DATA,("Adress adsr=0x%x",GPIBin(adsr)));
	bdSendAuxCmd(AUX_GTS);	/* go to standby */

	if (!noTimo) {
		DBGprint(DBG_BRANCH, ("timeout  "));
		ibsta |= (ERR | TIMO);
		iberr = EABO;
	}
	DBGprint(DBG_BRANCH, ("done  "));
	GPIBout(imr0, 0);		/* clear COIE if set */
	ibcnt = bytes;
	DBGout();

}









