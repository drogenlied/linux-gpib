#include <ibsys.h>
#include <board.h>

#if DMAOP
/*
 * Start a DMA operation and wait on its completion.
 */

IBLCL int osDoDMA(ibio_op_t *rwop)
{
	int		resid;		/* residual transfer count */

	DBGin("osDoDMA");

	rwop->io_pbuf = (uint32) rwop->io_vbuf;

	DBGprint(DBG_DATA, ("pbuf=0x%x cnt=%d  ", (unsigned int)rwop->io_pbuf, rwop->io_cnt));


	/* program dma controller */

	cli();
        disable_dma( ibdma );
	clear_dma_ff ( ibdma );
	set_dma_count( ibdma, rwop->io_cnt );
	set_dma_addr ( ibdma, rwop->io_pbuf);

	if (rwop->io_flags & IO_READ) {
		DBGprint(DBG_BRANCH, ("enabling DMA READ  "));
		set_dma_mode( ibdma, DMA_MODE_READ );
	}
	else {
		DBGprint(DBG_BRANCH, ("enabling DMA WRITE  "));
		set_dma_mode( ibdma, DMA_MODE_WRITE );
	}

#ifdef NIAT
	GPIBout(dmaEn, (GPIBin(dmaEn) | HR_DMAEN));
#endif

	enable_dma( ibdma );/* enable Host side DMA transfers */
	sti();

	bdDMAstart(rwop);
	bdDMAwait(rwop, 0);
	resid = bdDMAstop(rwop);

        disable_dma( ibdma );/* disable DMA transfers */
#ifdef NIAT
	GPIBout(dmaEn, (GPIBin(dmaEn) & ~HR_DMAEN));
#endif
	DBGout();
	return resid;
}

#endif /* dmaop */



