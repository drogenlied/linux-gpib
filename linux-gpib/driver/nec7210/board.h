
#ifndef _GPIB_PCIIA_BOARD_H
#define _GPIB_PCIIA_BOARD_H

#include <gpibP.h>
#include <asm/io.h>
#include <gpib_buffer.h>

extern ssize_t nec7210_read(uint8_t *buffer, size_t length, uint8_t eos);
extern ssize_t nec7210_write(uint8_t *buffer, size_t length, int send_eoi);
extern ssize_t nec7210_command(uint8_t *buffer, size_t length);
extern int nec7210_take_control(int syncronous);
extern int nec7210_go_to_standby(void);
extern void nec7210_interface_clear(int assert);
extern unsigned int nec7210_wait(unsigned int status_mask);
extern void nec7210_remote_enable(int enable);
extern void nec7210_enable_eos(uint8_t eos_bytes, int compare_8_bits);
extern void nec7210_disable_eos(void);
extern unsigned int nec7210_update_status(void);

extern void bdSendAuxCmd(int cmd);

extern unsigned long ibbase;	/* base addr of GPIB interface registers  */
extern unsigned long remapped_ibbase;	// ioremapped memory io address

extern unsigned int ibirq;	/* interrupt request line for GPIB (1-7)  */
extern unsigned int ibdma ;      /* DMA channel                            */
extern struct pci_dev *ib_pci_dev;	// pci_dev for plug and play boards

extern volatile int noTimo;     /* timeout flag */
extern int          pgmstat;    /* Program state */
extern int          auxa_bits;  /* static bits for AUXRA (EOS modes) */

extern gpib_buffer_t *read_buffer, *write_buffer;

// interrupt service routine
void nec7210_interrupt(int irq, void *arg, struct pt_regs *registerp);

// boolean values that signal various conditions
extern volatile int write_in_progress;	// data can be sent
extern volatile int command_out_ready;	// command can be sent
extern volatile int dma_transfer_complete;	// dma transfer is done

extern wait_queue_head_t nec7210_write_wait;
extern wait_queue_head_t nec7210_command_wait;
extern wait_queue_head_t nec7210_read_wait;
extern wait_queue_head_t nec7210_status_wait;

// software copies of bits written to interrupt mask registers
extern volatile int imr1_bits, imr2_bits;

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

