
#ifndef _GPIB_BOARD_H
#define _GPIB_BOARD_H

#include <asm/io.h>
#include <gpibP.h>

extern unsigned long ibbase;	/* base addr of GPIB interface registers  */
extern unsigned long remapped_ibbase;	// ioremapped base address for memory mapped boards
extern unsigned int ibirq;	/* interrupt request line for GPIB (1-7)  */
extern unsigned int ibdma ;      /* DMA channel                            */
extern struct pci_dev *ib_pci_dev;	// pci_dev for plug and play boards

#define CBI_ISA_GPIB         0
#define CBI_ISA_GPIB_LC      1
#define CBI_PCM_GPIB         2
#define CBI_PCI_GPIB         3


extern uint8       board_type;


extern volatile int noTimo;     /* timeout flag */
extern int          pgmstat;    /* Program state */
extern int          auxrabits;  /* static bits for AUXRA (EOS modes) */

extern uint8       CurHSMode;

#define FIFO_SIZE 1024

#define LOW_PORT 0x2e1

/* this routines are 'wrappers' for the outb() macros */

/*
 * Input a one-byte value from the specified I/O port
 */
extern inline uint8_t bdP8in(unsigned long in_addr)
{
//#if defined(CBI_PCI)
//	return readb(remapped_ibbase + in_addr);
//#else
	return inb(ibbase + in_addr);
//#endif
}


/*
 * Output a one-byte value to the specified I/O port
 */
extern inline void bdP8out(unsigned long out_addr, uint8_t out_value)
{
//#if defined(CBI_PCI)
//	writeb(out_value, remapped_ibbase + out_addr);
//#else
	outb(out_value, ibbase + out_addr);
//#endif
}


#endif	// _GPIB_BOARD_H
