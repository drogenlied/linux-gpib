
#include <ibsys.h>
#include <asm/io.h>

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

