
#include <gpibP.h>

extern uint16      ibbase;	/* base addr of GPIB interface registers  */
extern uint8       ibirq;	/* interrupt request line for GPIB (1-7)  */
extern uint8       ibdma ;      /* DMA channel                            */


extern ibregs_t *ib;            /* Local pointer to IB registers */


extern volatile int noTimo;     /* timeout flag */
extern int          pgmstat;    /* Program state */
extern int          auxrabits;  /* static bits for AUXRA (EOS modes) */

#define IB ib

/* this routines are 'wrappers' for the outb() macros */

/*
 * Input a one-byte value from the specified I/O port
 */
extern inline uint8 bdP8in(void * in_addr)
{
	uint8		retval;

	retval = osP8in((unsigned int) in_addr);

	return retval;
}


/*
 * Output a one-byte value to the specified I/O port
 */

extern inline void bdP8out(void * out_addr, uint8 out_value)
{
	osP8out((unsigned int) out_addr,out_value);
}

/*
 * Input a two-byte value from the specified I/O port
 */
extern inline uint16 bdP16in(void * in_addr)
{
	uint16		retval;

	retval = osP16in((unsigned int) in_addr);

	return retval;
}


/*
 * Output a two-byte value to the specified I/O port
 */
extern inline void bdP16out(void * out_addr, uint16 out_value)
{
       osP16out((unsigned int) out_addr,out_value);
}







