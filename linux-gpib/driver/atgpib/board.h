
#include <gpibP.h>

extern unsigned int      ibbase;	/* base addr of GPIB interface registers  */
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
	return inb_p((unsigned int) in_addr);
}

/*
 * Output a one-byte value to the specified I/O port
 */

extern inline void bdP8out(void * out_addr, uint8 out_value)
{
	outb_p(out_value, (unsigned int) out_addr);
}

/*
 * Input a two-byte value from the specified I/O port
 */
extern inline uint16 bdP16in(void * in_addr)
{
	return inw_p((unsigned int) in)addr);
}

/*
 * Output a two-byte value to the specified I/O port
 */
extern inline void bdP16out(void * out_addr, uint16 out_value)
{
	outw_p(out_value, (unsigned int) out_addr);
}

