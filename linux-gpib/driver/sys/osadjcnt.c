#include <ibsys.h>


/*
 * Adjust I/O count to the maximum transferable value (DMA).
 */
IBLCL void osDMAAdjCnt(ibio_op_t *rwop)
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
 * Adjust I/O count to the maximum transferable value (PIO).
 */
IBLCL void osPIOAdjCnt(ibio_op_t *rwop)
{
	DBGin("osAdjCnt");
	DBGprint(DBG_BRANCH, ("NOP  "));
	DBGout();
}
