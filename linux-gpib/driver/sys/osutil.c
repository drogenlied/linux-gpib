
#include <ibsys.h>
#include <asm/io.h>

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

void writeb_wrapper( unsigned int value, unsigned long address )
{
	writeb( value, address );
};

unsigned int readb_wrapper( unsigned long address )
{
	return readb( address );
};

void outb_wrapper( unsigned int value, unsigned long address )
{
	outb( value, address );
};

unsigned int inb_wrapper( unsigned long address )
{
	return inb( address );
};

EXPORT_SYMBOL( writeb_wrapper );
EXPORT_SYMBOL( readb_wrapper );
EXPORT_SYMBOL( outb_wrapper );
EXPORT_SYMBOL( inb_wrapper );

