
#ifndef _GPIB_PCIIA_BOARD_H
#define _GPIB_PCIIA_BOARD_H

#include <gpibP.h>
#include <asm/io.h>

extern unsigned long ibbase;	/* base addr of GPIB interface registers  */
extern unsigned long remapped_ibbase;	// ioremapped memory io address

extern unsigned int ibirq;	/* interrupt request line for GPIB (1-7)  */
extern unsigned int ibdma ;      /* DMA channel                            */
extern struct pci_dev *ib_pci_dev;	// pci_dev for plug and play boards

extern volatile int noTimo;     /* timeout flag */
extern int          pgmstat;    /* Program state */
extern int          auxrabits;  /* static bits for AUXRA (EOS modes) */

extern void *ib;            /* Local pointer to IB registers */

// interrupt service routine
void nec7210_interrupt(int irq, void *arg, struct pt_regs *registerp);

#define LOW_PORT 0x2e1

/* this routines are 'wrappers' for the outb() macros */

/*
 * Input a one-byte value from the specified I/O port
 */

extern inline uint8_t bdP8in(unsigned long in_addr)
{
#if defined(MODBUS_PCI)
	return readw(remapped_ibbase + in_addr) & 0xff;
#else
	return inb_p(ibbase + in_addr);
#endif
}


/*
 * Output a one-byte value to the specified I/O port
 */
extern inline void bdP8out(unsigned long out_addr, uint8_t out_value)
{
#if defined(MODBUS_PCI)
	writeb(out_value, remapped_ibbase + out_addr );
#else
	outb_p(out_value, ibbase + out_addr);
#endif
}

/************************************************************************/

#endif	//_GPIB_PCIIA_BOARD_H

