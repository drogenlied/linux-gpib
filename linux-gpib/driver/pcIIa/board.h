
#ifndef _GPIB_PCIIA_BOARD_H
#define _GPIB_PCIIA_BOARD_H

#include <gpibP.h>
#include <asm/io.h>

extern unsigned int      ibbase;	/* base addr of GPIB interface registers  */
extern uint8       ibirq;	/* interrupt request line for GPIB (1-7)  */
extern uint8       ibdma ;      /* DMA channel                            */
extern struct pci_dev *ib_pci_dev;	// pci_dev for plug and play boards	

extern volatile int noTimo;     /* timeout flag */
extern int          pgmstat;    /* Program state */
extern int          auxrabits;  /* static bits for AUXRA (EOS modes) */

extern ibregs_t *ib;            /* Local pointer to IB registers */
#define IB ib


#define LOW_PORT 0x2e1

/* this routines are 'wrappers' for the outb() macros */

/*
 * Input a one-byte value from the specified I/O port
 */

extern unsigned int pci_base_reg;

#if defined(MODBUS_PCI)
extern inline uint8 bdP8in(faddr_t in_addr)
{
	int		retval;
        unsigned int         addr = pci_base_reg + ( (unsigned long) in_addr << 0x1 );
	retval = readw(addr);
	return (uint8) (retval & 0xff );
}
#else
extern inline uint8 bdP8in(faddr_t in_addr)
{
	uint8		retval;
#if !defined(NIPCIIa)
	retval = inb_p((unsigned int) in_addr);
#else
	retval = inb_p( ((unsigned int)in_addr << 10) | ibbase  );
#endif
	return retval;
}
#endif


/*
 * Output a one-byte value to the specified I/O port
 */
#if defined(MODBUS_PCI)
extern inline void bdP8out(faddr_t out_addr, uint8 out_value)
{
        unsigned int addr = pci_base_reg + ((unsigned long) out_addr << 0x1 );
	writeb(out_value, addr );
}
#else
extern inline void bdP8out(out_addr, out_value)
short out_addr;
uint8 out_value;
{
#if !defined(NIPCIIa)
	outb_p(out_value, out_addr);
#else
	outb_p(out_value, ((out_addr << 10) | ibbase ));
#endif
}
#endif

/************************************************************************/

#endif	//_GPIB_PCIIA_BOARD_H

