
#ifndef _GPIB_BOARD_H
#define _GPIB_BOARD_H

#include <gpibP.h>
#include <asm/io.h>

extern unsigned long ibbase;	/* base addr of GPIB interface registers  */
extern unsigned int ibirq;	/* interrupt request line for GPIB (1-7)  */
extern unsigned int ibdma ;      /* DMA channel                            */

extern volatile int noTimo;     /* timeout flag */
extern int pgmstat;    /* Program state */
extern int auxrabits;  /* static bits for AUXRA (EOS modes) */

/* this routines are 'wrappers' for the outb() macros */

/*
 * Input a one-byte value from the specified I/O port
 */
extern inline uint8_t bdP8in(unsigned long in_addr)
{
	return inb_p(ibbase + in_addr);
}

/*
 * Output a one-byte value to the specified I/O port
 */

extern inline void bdP8out(unsigned long out_addr, uint8_t out_value)
{
	outb_p(out_value, ibbase + out_addr);
}

/*
 * Input a two-byte value from the specified I/O port
 */
extern inline uint16_t bdP16in(unsigned long in_addr)
{
	return inw_p(ibbase + in_addr);
}

/*
 * Output a two-byte value to the specified I/O port
 */
extern inline void bdP16out(unsigned long out_addr, uint16_t out_value)
{
	outw_p(out_value, ibbase + out_addr);
}

#endif	// _GPIB_BOARD_H
