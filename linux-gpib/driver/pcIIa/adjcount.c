#include <board.h>

/*
 *  BdAdjCnt (DMA)
 *  This function adjusts the I/O count to the maximum transferable value.
 */
IBLCL void bdDMAAdjCnt(ibio_op_t *rwop)
{ 
	DBGin("bdAdjCnt(dma)");
	bdPIOAdjCnt(rwop);
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

