#ifndef _GPIB_BOARD_H
#define _GPIB_BOARD_H

#include <gpibP.h>
#include <asm/io.h>

extern unsigned long ibbase;	/* base addr of GPIB interface registers  */
extern unsigned long remapped_ibbase;	// ioremapped address for memory mapped io
extern unsigned int ibirq;	/* interrupt request line for GPIB (1-7)  */
extern unsigned int ibdma ;      /* DMA channel                            */

extern volatile int noTimo;     /* timeout flag */
extern int          pgmstat;    /* Program state */
extern int          ccrbits;    /* static bits for card control register */

#if DMAOP
#error This board does not support DMA (not implemented yet)
#endif

/* this routines are 'wrappers' for the outb() macros */

/*
 * Input a one-byte value from the specified I/O port
 */
extern inline uint8_t bdP8in(unsigned long in_addr)
{
#if defined(HP82335)
	return readb(remapped_ibbase + in_addr);
#else
	return inb_p(ibbase + in_addr);
#endif
}

/*
 * Output a one-byte value to the specified I/O port
 */
extern inline void bdP8out(unsigned long out_addr, uint8_t out_value)
{
#if defined(HP82335)
	writeb(out_value, remapped_ibbase + out_addr);
#else
        outb_p(out_value, ibbase + out_addr);
#endif
}

#endif	// _GPIB_BOARD_H







