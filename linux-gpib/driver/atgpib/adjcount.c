#include "board.h"

#if DMAOP

/*
 * Adjust I/O count to the maximum transferable value (DMA).
 */
void osAdjCnt(ibio_op_t *rwop)
{

	unsigned	cnt_to_pgbrk;

	DBGin("osAdjCnt");
	if (((unsigned) rwop->io_vbuf) & (DMA_PAGE_SIZE - 1))
		cnt_to_pgbrk = (-((unsigned) rwop->io_vbuf)) & (DMA_PAGE_SIZE - 1);
	else
		cnt_to_pgbrk = DMA_PAGE_SIZE;

	DBGprint(DBG_DATA, ("cnt_to_pgbrk=%d (0x%x)  ", cnt_to_pgbrk, cnt_to_pgbrk));
	if (cnt_to_pgbrk < rwop->io_cnt)
		rwop->io_cnt = cnt_to_pgbrk;
	DBGout();

}

/*
 *  BdAdjCnt (DMA)
 *  This function adjusts the I/O count to the maximum transferable value.
 */
IBLCL void bdAdjCnt(ibio_op_t *rwop)
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

#else

/*
 *  BdAdjCnt (PIO)
 *  This function adjusts the I/O count to the maximum transferable value.
 */
IBLCL void bdAdjCnt(ibio_op_t *rwop)
{
	unsigned	requested_cnt;

	DBGin("bdAdjCnt");
	rwop->io_flags |= IO_LAST;/* start off assuming this is the last chunk */
	requested_cnt = rwop->io_cnt;
	if (rwop->io_cnt > IBMAXIO)
		rwop->io_cnt = IBMAXIO;
	if (requested_cnt != rwop->io_cnt) {
		rwop->io_flags &= ~IO_LAST;
		DBGprint(DBG_DATA, ("adj_io_cnt:%d  ", rwop->io_cnt));
	}
	DBGout();
}
#endif	// DMAOP