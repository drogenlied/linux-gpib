#include <board.h>
/*
 *  BdDMAwait
 *  This function checks or waits for "DMA operation complete".
 */
IBLCL int bdDMAwait(ibio_op_t *rwop, int noWait)
{
	DBGin("bdDMAwait");
	if (rwop->io_flags & IO_READ)
	    while (!(GPIBin(isr3) & HR_DONE) && NotTimedOut()) {
		if (noWait) {
			DBGout();
			return 0;
		}
		WaitingFor(HR_DONE);
	}
	else while (!(GPIBin(isr3) & (HR_DONE | HR_TLCI)) && NotTimedOut()) {
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
		GPIBout(imr2, HR_DMAI);
		GPIBout(cmdr, GO);
		GPIBout(imr1, HR_ENDIE);
	}
	else {
		GPIBout(imr1, HR_ERRIE);/* set imr1 before imr2 */
		GPIBout(imr2, HR_DMAO);
		GPIBout(cmdr, GO);
	}
	DBGout();
}


/*
 *  BdDMAstop
 *  This function halts the DMA controller.
 */
IBLCL int bdDMAstop(ibio_op_t *rwop)
{
	uint8	lsb;			/* unsigned residual LSB */
	char	msb;			/* signed residual MSB */
	int	resid;			/* signed residual value */

	DBGin("bdDMAstop");
	GPIBout(cmdr, STOP);
	GPIBout(imr3, 0);
	lsb = GPIBin(cntl);
	msb = GPIBin(cnth);
	resid = -(((int) lsb) | (((int) msb) << 8));
	DBGprint(DBG_DATA, ("FIFOresid=%d  ", resid));
	DBGout();
	return resid;
}

