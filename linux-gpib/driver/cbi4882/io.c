#include <board.h>
#include <asm/io.h>

#define LOW_PORT 0x2e1

/* this routines are 'wrappers' for the outb() macros */

/*
 * Input a one-byte value from the specified I/O port
 */
inline uint8 bdP8in(in_addr)
short in_addr;
{
	uint8		retval;
#if defined(CBI_PCI1)
	retval = readb(in_addr);

#else
	retval = osP8in(in_addr);
#endif
	return retval;
}


/*
 * Output a one-byte value to the specified I/O port
 */
inline void bdP8out(out_addr, out_value)	
short out_addr;
uint8 out_value;
{

#if defined(CBI_PCI1)
  writeb(out_value,out_addr);

#else
	osP8out(out_addr,out_value);
#endif
}


/************************************************************************/

#if 0

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


#endif

