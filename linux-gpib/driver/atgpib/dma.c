#include <ibsys.h>
#include "board.h"

/*
 *  BdDMAwait
 *  This function checks or waits for "DMA operation complete".
 */
IBLCL int bdDMAwait(ibio_op_t *rwop, int noWait)
{
	DBGin("bdDMAwait");
	if (rwop->io_flags & IO_READ)
	    while (!(GPIBin(ISR3) & HR_DONE) && NotTimedOut()) {
		if (noWait) {
			DBGout();
			return 0;
		}
		WaitingFor(HR_DONE);
	}
	else while (!(GPIBin(ISR3) & (HR_DONE | HR_TLCI)) && NotTimedOut()) {
		if (noWait) {
			DBGout();
			return 0;
		}
		WaitingFor(HR_TLCI | HR_DONE);
	}
	DBGout();
	return 1;
}

/*
 *  BdDMAstart
 *  This function configures and starts the DMA controller.
 */
IBLCL void bdDMAstart(ibio_op_t *rwop)
{
	DBGin("bdDMAstart");
	if (rwop->io_flags & IO_READ) {
		GPIBout(IMR2, HR_DMAI);
		GPIBout(CMDR, GO);
		GPIBout(IMR1, HR_ENDIE);
	}
	else {
		GPIBout(IMR1, HR_ERRIE);/* set IMR1 before IMR2 */
		GPIBout(IMR2, HR_DMAO);
		GPIBout(CMDR, GO);
	}
	DBGout();
}


/*
 *  BdDMAstop
 *  This function halts the DMA controller.
 */
IBLCL int bdDMAstop(ibio_op_t *rwop)
{
	uint8_t	lsb;			/* unsigned residual LSB */
	int8_t	msb;			/* signed residual MSB */
	int	resid;			/* signed residual value */

	DBGin("bdDMAstop");
	GPIBout(CMDR, STOP);
	GPIBout(IMR3, 0);
	lsb = GPIBin(CNTL);
	msb = GPIBin(CNTH);
	resid = -(((int) lsb) | (((int) msb) << 8));
	DBGprint(DBG_DATA, ("FIFOresid=%d  ", resid));
	DBGout();
	return resid;
}


#if DMAOP
/*
 * Start a DMA operation and wait on its completion.
 */

IBLCL int osDoDMA(ibio_op_t *rwop)
{
	unsigned long flags;
	int		resid;		/* residual transfer count */

	DBGin("osDoDMA");

	rwop->io_pbuf = virt_to_bus(rwop->io_vbuf);

	DBGprint(DBG_DATA, ("pbuf=0x%x cnt=%d  ", (unsigned int)rwop->io_pbuf, rwop->io_cnt));


	/* program dma controller */

	flags = claim_dma_lock();
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

	GPIBout(DMA_EN, (GPIBin(DMA_EN) | HR_DMAEN));

	enable_dma( ibdma );/* enable Host side DMA transfers */
	release_dma_lock(flags);

	bdDMAstart(rwop);
	bdDMAwait(rwop, 0);
	resid = bdDMAstop(rwop);

	disable_dma( ibdma );/* disable DMA transfers */
	GPIBout(DMA_EN, (GPIBin(DMA_EN) & ~HR_DMAEN));
	DBGout();
	return resid;
}

#endif /* dmaop */




