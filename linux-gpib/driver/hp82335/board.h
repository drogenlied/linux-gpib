
#include <gpibP.h>
#include <asm/io.h>

extern unsigned int      ibbase;	/* base addr of GPIB interface registers  */
extern uint8       ibirq;	/* interrupt request line for GPIB (1-7)  */
extern uint8       ibdma ;      /* DMA channel                            */

extern volatile int noTimo;     /* timeout flag */
extern int          pgmstat;    /* Program state */
extern int          ccrbits;    /* static bits for card control register */

extern ibregs_t *ib;            /* Local pointer to IB registers */
#define IB ib

#if DMAOP
#error This board does not support DMA (not implemented yet)
#endif

/* this routines are 'wrappers' for the outb() macros */
/* The HP82335 uses a memory area so outb/inb is not sufficient here */

/*
 * Input a one-byte value from the specified I/O port
 */
extern inline uint8 bdP8in(faddr_t in_addr)
{
#if defined(HP82335)
	return readb((unsigned int) in_addr);
#else
	return inb_p((unsigned int) in_addr);
#endif
}

/*
 * Output a one-byte value to the specified I/O port
 */
extern inline void bdP8out(faddr_t out_addr, uint8 out_value)
{
#if defined(HP82335)
	writeb(out_value, (unsigned int) out_addr);
#else
        outb_p(out_value, (unsigned int)out_addr);
#endif
}








