
#ifndef _GPIB_PCIIA_BOARD_H
#define _GPIB_PCIIA_BOARD_H

#include <gpibP.h>
#include <asm/io.h>

extern uint16      ibbase;	/* base addr of GPIB interface registers  */
extern uint8       ibirq;	/* interrupt request line for GPIB (1-7)  */
extern uint8       ibdma ;      /* DMA channel                            */

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
extern inline uint8 bdP8in(in_addr)
faddr_t in_addr;
{
	int		retval;
        faddr_t         addr = pci_base_reg + ( (unsigned long) in_addr << 0x1 );
	retval = readw(addr);
	printk("INB: 0x%x = 0x%x off=0x%x\n",addr,(retval & 0xff),in_addr);
	return (uint8) (retval & 0xff );
}
#else
extern inline uint8 bdP8in(in_addr)
short in_addr;
{
	uint8		retval;
#if !defined(NIPCIIa)
	retval = osP8in(in_addr);
#else
	retval = osP8in( (in_addr << 10) | ibbase  );
#endif
	return retval;
}
#endif


/*
 * Output a one-byte value to the specified I/O port
 */
#if defined(MODBUS_PCI)
extern inline void bdP8out(out_addr, out_value)
faddr_t out_addr;
uint8 out_value;
{
        faddr_t addr = pci_base_reg + ((unsigned long) out_addr << 0x1 );
	printk("OUTB: 0x%x , 0x%x\n",addr ,out_value);
	writeb(out_value, addr );
}
#else
extern inline void bdP8out(out_addr, out_value)
short out_addr;
uint8 out_value;
{
#if !defined(NIPCIIa)
	osP8out(out_addr,out_value);
#else
        /*printk("bdP8out: offset = %d addr=0x%x \n",out_addr ,((out_addr << 10) | ibbase ) );*/
	osP8out(((out_addr << 10) | ibbase ),out_value);
#endif
}
#endif

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




#if 0
#if DMAOP

#error board does not support DMA

#endif
#endif

#endif	//_GPIB_PCIIA_BOARD_H

