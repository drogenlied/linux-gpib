#include <board.h>

/* this routines are 'wrappers' for the outb() macros */

/*
 * Input a one-byte value from the specified I/O port
 */
IBLCL uint8 bdP8in(short in_addr)
{
	uint8		retval;

	retval = osP8in(in_addr);

	return retval;
}


/*
 * Output a one-byte value to the specified I/O port
 */

IBLCL void bdP8out(short out_addr, uint8 out_value)
{
	osP8out(out_addr,out_value);
}

/*
 * Input a two-byte value from the specified I/O port
 */
IBLCL uint16 bdP16in(short in_addr)
{
	uint16		retval;

	retval = osP16in(in_addr);

	return retval;
}


/*
 * Output a two-byte value to the specified I/O port
 */
IBLCL void bdP16out(short out_addr, uint16 out_value)
{
       osP16out(out_addr,out_value);
}



