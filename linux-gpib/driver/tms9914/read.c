
#include "board.h"

extern int eosmodes;

IBLCL void bdread(ibio_op_t *rdop)
{
	faddr_t		buf;
	unsigned	cnt;
	int8_t		s1;		/* software copies of HW status regs */
        uint8_t           eos;


	DBGin("bdread");

	buf = rdop->io_vbuf;
	cnt = rdop->io_cnt;
	DBGprint(DBG_DATA, ("buf=0x%p cnt=%d  ", buf, cnt));

	eos = bdGetEOS();
	DBGprint(DBG_DATA, ("eos=0x%x",eos));

	DBGprint(DBG_BRANCH, ("begin PIO loop  "));
	while (ibcnt < cnt ) 
	  { 
	    if (pgmstat & PS_HELD) GPIBout(AUXCR,AUX_RHDF);
	    else pgmstat |= PS_HELD;
	    
	    if( (s1=bdWaitIn()) < 0 ) 
	      {
		if (!noTimo) /* Read timed out */
		  break;
		/* Read ended with EOI, so read last byte */
		buf[ibcnt++] = GPIBin(DIR);
		DBGprint(DBG_DATA,("buf[%d]='%u' / ISR0=0x%x\n",ibcnt-1,buf[ibcnt-1],s1));
		break;
	      }

	    buf[ibcnt++] = GPIBin(DIR);
	    DBGprint(DBG_DATA,("buf[%d]='%u' / ISR0=0x%x\n",ibcnt-1,buf[ibcnt-1],s1));
	    
	    if((eosmodes & REOS) && (buf[ibcnt-1] == eos ))  
	      break;
	  }
	DBGprint(DBG_BRANCH, ("done  "));
	ibsta |= END;

	if (!noTimo) 
	  {
	    /* in case of timeout, driver is not in holdoff-state
	     * (RHDF sended, but no byte recieved)
	     * Driver must NOT send twice RHDF!!
	     */
	    pgmstat &= ~PS_HELD;
	    ibsta |= (ERR | TIMO);
	    iberr = EABO;
	  }

	DBGout();
}









