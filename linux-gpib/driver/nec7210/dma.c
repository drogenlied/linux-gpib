#include <board.h>

#if DMAOP

// useless crap

/*
 *  BdDMAwait
 *  This function checks or waits for "DMA operation complete".
 */
IBLCL void bdDMAwait(ibio_op_t *rwop, int noWait)
{
}

/*
 *  BdDMAstart
 *  This function configures and starts the DMA controller.
 */
IBLCL void bdDMAstart(ibio_op_t *rwop)
{
}

IBLCL int bdDMAstop(ibio_op_t *rwop)
{
        return 0;
}

#endif /* DMAOP */





