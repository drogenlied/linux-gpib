#include <board.h>

/* this routines are 'wrappers' for the outb() macros */

/*
 * Input a one-byte value from the specified I/O port
 */
inline uint8 bdP8in(in_addr)
short in_addr;
{
	uint8		retval;

	retval = osP8in(in_addr);

	return retval;
}


/*
 * Output a one-byte value to the specified I/O port
 */
inline void bdP8out(out_addr, out_value)	
short out_addr;
uint8 out_value;
{
	osP8out(out_addr,out_value);
}

/*
 * Input a two-byte value from the specified I/O port
 */
inline uint16 bdP16in(in_addr)
short in_addr;
{
	uint16		retval;

	retval = osP16in(in_addr);

	return retval;
}


/*
 * Output a two-byte value to the specified I/O port
 */
inline void bdP16out(out_addr, out_value)	
short out_addr;
uint16 out_value;
{
       osP16out(out_addr,out_value);
}



