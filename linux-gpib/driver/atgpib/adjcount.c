#include <board.h>


/* @BEGIN-MAN

\routine{ bdAdjCnt( ibio_op_t *rwop )  }
%\synopsis{  }
\description{ Adjusts the I/O count to the Maximum transferable Value. If DMA Mode has been disabled the routine does not have much to do. }
%\returns{   }
%\bugs{   }

   @END-MAN */


/*
 *  BdAdjCnt (DMA)
 *  This function adjusts the I/O count to the maximum transferable value.
 */
IBLCL void bdDMAAdjCnt(ibio_op_t *rwop)
{ 
	unsigned	requested_cnt;

	DBGin("bdAdjCnt(dma)");
	rwop->io_flags |= IO_LAST;	/* start off assuming this is the last chunk */
	if (rwop->io_cnt > 1) {
		requested_cnt = rwop->io_cnt;
		if (((int) rwop->io_vbuf) & 1)
					/* ...buffer address is odd? */
			rwop->io_cnt = 1;
		else {
			if (rwop->io_cnt > IBMAXIO)
				rwop->io_cnt = IBMAXIO;
					/* ...IBMAXIO is assumed always to be even */
			else if (rwop->io_cnt & 1)
				rwop->io_cnt--;
					/* ...make odd count even */

			osAdjCnt(rwop);
		}
		if (requested_cnt != rwop->io_cnt) {
			rwop->io_flags &= ~IO_LAST;
			DBGprint(DBG_DATA, ("adj_io_cnt:%d  ", rwop->io_cnt));
		}
	}
	DBGout();
}


/*
 *  BdAdjCnt (PIO)
 *  This function adjusts the I/O count to the maximum transferable value.
 */
IBLCL void bdPIOAdjCnt(ibio_op_t *rwop)
{ 
	unsigned	requested_cnt;

	DBGin("bdAdjCnt");
	rwop->io_flags |= IO_LAST;/* start off assuming this is the last chunk */
	requested_cnt = rwop->io_cnt;
	if (rwop->io_cnt > IBMAXIO)
		rwop->io_cnt = IBMAXIO;
	osAdjCnt(rwop);
	if (requested_cnt != rwop->io_cnt) {
		rwop->io_flags &= ~IO_LAST;
		DBGprint(DBG_DATA, ("adj_io_cnt:%d  ", rwop->io_cnt));
	}
	DBGout();
}
