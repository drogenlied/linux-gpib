
#include "ibsys.h"
#include <asm/io.h>

void writeb_wrapper( unsigned int value, unsigned long address )
{
	writeb( value, address );
};

void writew_wrapper( unsigned int value, unsigned long address )
{
	writew( value, address );
};

unsigned int readb_wrapper( unsigned long address )
{
	return readb( address );
};

unsigned int readw_wrapper( unsigned long address )
{
	return readw( address );
};

void outb_wrapper( unsigned int value, unsigned long address )
{
	outb( value, address );
};

void outw_wrapper( unsigned int value, unsigned long address )
{
	outw( value, address );
};

unsigned int inb_wrapper( unsigned long address )
{
	return inb( address );
};

unsigned int inw_wrapper( unsigned long address )
{
	return inw( address );
};

EXPORT_SYMBOL( writeb_wrapper );
EXPORT_SYMBOL( readb_wrapper );
EXPORT_SYMBOL( outb_wrapper );
EXPORT_SYMBOL( inb_wrapper );
EXPORT_SYMBOL( writew_wrapper );
EXPORT_SYMBOL( readw_wrapper );
EXPORT_SYMBOL( outw_wrapper );
EXPORT_SYMBOL( inw_wrapper );

