#include <asm/io.h>
#include <gpibP.h>

extern unsigned int      ibbase;	/* base addr of GPIB interface registers  */
extern uint8       ibirq;	/* interrupt request line for GPIB (1-7)  */
extern uint8       ibdma ;      /* DMA channel                            */
extern struct pci_dev *ib_pci_dev;	// pci_dev for plug and play boards

#define CBI_ISA_GPIB         0
#define CBI_ISA_GPIB_LC      1
#define CBI_PCM_GPIB         2
#define CBI_PCI_GPIB         3


extern uint8       board_type;


extern volatile int noTimo;     /* timeout flag */
extern int          pgmstat;    /* Program state */
extern int          auxrabits;  /* static bits for AUXRA (EOS modes) */

extern ibregs_t *ib;            /* Local pointer to IB registers */
#define IB ib

extern uint8       CurHSMode;

#define FIFO_SIZE 1024

#if 0
#if DMAOP

#error board does not support DMA

#endif
#endif

#define LOW_PORT 0x2e1

/* this routines are 'wrappers' for the outb() macros */

/*
 * Input a one-byte value from the specified I/O port
 */
extern inline uint8 bdP8in(faddr_t in_addr)
{
#if defined(CBI_PCI)
	return readb((unsigned int)in_addr);
#else
	return inb((unsigned int)in_addr);
#endif
}


/*
 * Output a one-byte value to the specified I/O port
 */
extern inline void bdP8out(faddr_t out_addr, uint8 out_value)
{
#if defined(CBI_PCI)
	writeb(out_value, (unsigned int) out_addr);
#else
	outb(out_value, (unsigned int) out_addr);
#endif
}


/************************************************************************/

