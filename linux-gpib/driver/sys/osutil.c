
#include <ibsys.h>

void osChngBase(gpib_board_t *board, unsigned long new_base)
{
	board->ibbase = new_base;
}

void osChngIRQ(gpib_board_t *board, int new_irq)
{
	board->ibirq = new_irq;
}

void osChngDMA(gpib_board_t *board, int new_dma)
{
	board->ibdma = new_dma;
}


