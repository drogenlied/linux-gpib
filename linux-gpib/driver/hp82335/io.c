#include <board.h>

/* this routines are 'wrappers' for the outb() macros */
/* The HP82335 uses a memory area so outb/inb is not sufficient here */

/*
 * Input a one-byte value from the specified I/O port
 */
inline uint8 bdP8in(in_addr)
faddr_t in_addr;
{
	uint8		retval;
#if defined(HP82335)
	retval = *in_addr;
#endif
#if defined(TMS9914)
	retval = osP8in(in_addr);
#endif

	return retval;
}


/*
 * Output a one-byte value to the specified I/O port
 */
inline void bdP8out(out_addr, out_value)	
faddr_t out_addr;
uint8 out_value;
{
#if defined(HP82335)
	*out_addr = out_value;
#endif
#if defined(TMS9914)
        osP8out(out_addr,out_value);
#endif

}


