/***************************************************************************
                                   nec7210.h
                             -------------------
    begin                : Jan 2002
    copyright            : (C) 2002 by Frank Mori Hess
    email                : fmhess@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _NEC7210_H
#define _NEC7210_H

#include <linux/types.h>
#include <linux/spinlock.h>
#include "gpib_types.h"

/* struct used to provide variables local to a nec7210 chip */
typedef struct nec7210_private_struct nec7210_private_t;
struct nec7210_private_struct
{
	unsigned long iobase;
	unsigned int offset;	// offset between successive nec7210 io addresses
	unsigned int dma_channel;
	uint8_t *dma_buffer;
	unsigned int dma_buffer_length;	// length of dma buffer
	dma_addr_t dma_buffer_addr;	// bus address of board->buffer for use with dma
	// software copy of bits written to registers
	volatile uint8_t reg_bits[ 8 ];
	volatile uint8_t auxa_bits;	// bits written to auxilliary register A
	volatile uint8_t auxb_bits;	// bits written to auxilliary register B
	// used to keep track of board's state, bit definitions given below
	volatile int state;
	// wrappers for outb, inb, readb, or writeb
	uint8_t (*read_byte)(nec7210_private_t *priv, unsigned int register_number);
	void (*write_byte)(nec7210_private_t *priv, uint8_t byte, unsigned int register_number);
};

// slightly shorter way to access read_byte and write_byte
static inline uint8_t read_byte(nec7210_private_t *priv, unsigned int register_number)
{
	return priv->read_byte(priv, register_number);
}
static inline void write_byte(nec7210_private_t *priv, uint8_t byte, unsigned int register_number)
{
	priv->write_byte(priv, byte, register_number);
}

// nec7210_private_t.state bit numbers
enum
{
	PIO_IN_PROGRESS_BN,	// pio transfer in progress
	DMA_READ_IN_PROGRESS_BN,	// dma read transfer in progress
	DMA_WRITE_IN_PROGRESS_BN,	// dma write transfer in progress
	READ_READY_BN,	// board has data byte available to read
	WRITE_READY_BN,	// board is ready to send a data byte
	COMMAND_READY_BN,	// board is ready to send a command byte
	RECEIVED_END_BN,	// received END
	BUS_ERROR_BN,	// output error has occurred
	RFD_HOLDOFF_BN,	// rfd holdoff in effect
	DEV_CLEAR_BN,	// device clear received
};

// interface functions
ssize_t nec7210_read(gpib_board_t *board, nec7210_private_t *priv,
	uint8_t *buffer, size_t length, int *end);
ssize_t nec7210_write(gpib_board_t *board, nec7210_private_t *priv,
	uint8_t *buffer, size_t length, int send_eoi);
ssize_t nec7210_command(gpib_board_t *board, nec7210_private_t *priv,
	uint8_t *buffer, size_t length);
int nec7210_take_control(gpib_board_t *board, nec7210_private_t *priv,
	int syncronous);
int nec7210_go_to_standby(gpib_board_t *board, nec7210_private_t *priv);
void nec7210_request_system_control( gpib_board_t *board,
	nec7210_private_t *priv, int request_control );
void nec7210_interface_clear(gpib_board_t *board, nec7210_private_t *priv, int assert);
void nec7210_remote_enable(gpib_board_t *board, nec7210_private_t *priv, int enable);
void nec7210_enable_eos(gpib_board_t *board, nec7210_private_t *priv,
	uint8_t eos_bytes, int compare_8_bits);
void nec7210_disable_eos(gpib_board_t *board, nec7210_private_t *priv);
unsigned int nec7210_update_status( gpib_board_t *board, nec7210_private_t *priv,
	unsigned int clear_mask );
void nec7210_primary_address( const gpib_board_t *board,
	nec7210_private_t *priv, unsigned int address);
void nec7210_secondary_address( const gpib_board_t *board, nec7210_private_t *priv,
	unsigned int address, int enable);
int nec7210_parallel_poll(gpib_board_t *board, nec7210_private_t *priv, uint8_t *result);
void nec7210_serial_poll_response(gpib_board_t *board, nec7210_private_t *priv, uint8_t status);
void nec7210_parallel_poll_configure( gpib_board_t *board,
	nec7210_private_t *priv, unsigned int configuration );
void nec7210_parallel_poll_response( gpib_board_t *board,
	nec7210_private_t *priv, int ist );
uint8_t nec7210_serial_poll_status( gpib_board_t *board,
	nec7210_private_t *priv );
unsigned int nec7210_t1_delay( gpib_board_t *board,
	nec7210_private_t *priv, unsigned int nano_sec );
void nec7210_return_to_local( const gpib_board_t *board, nec7210_private_t *priv );

// utility functions
void nec7210_board_reset( nec7210_private_t *priv, const gpib_board_t *board );
void nec7210_board_online( nec7210_private_t *priv, const gpib_board_t *board );
unsigned int nec7210_set_reg_bits( nec7210_private_t *priv, unsigned int reg,
	unsigned int mask, unsigned int bits );
void nec7210_set_handshake_mode( gpib_board_t *board, nec7210_private_t *priv, int mode );
void nec7210_release_rfd_holdoff( gpib_board_t *board, nec7210_private_t *priv );
uint8_t nec7210_read_data_in( gpib_board_t *board, nec7210_private_t *priv, int *end );

// wrappers for io functions
uint8_t nec7210_ioport_read_byte(nec7210_private_t *priv, unsigned int register_num);
void nec7210_ioport_write_byte(nec7210_private_t *priv, uint8_t data, unsigned int register_num);
uint8_t nec7210_iomem_read_byte(nec7210_private_t *priv, unsigned int register_num);
void nec7210_iomem_write_byte(nec7210_private_t *priv, uint8_t data, unsigned int register_num);

// interrupt service routine
void nec7210_interrupt(gpib_board_t *board, nec7210_private_t *priv);
void nec7210_interrupt_have_status( gpib_board_t *board,
	nec7210_private_t *priv, int status1, int status2 );

// nec7210 has 8 registers
static const int nec7210_num_registers = 8;

/* nec7210 register numbers (might need to be multiplied by
 * a board-dependent offset to get actually io address offset)
 */
// write registers
enum nec7210_write_regs
{
	CDOR,	// command/data out
	IMR1,	// interrupt mask 1
	IMR2,	// interrupt mask 2
	SPMR,	// serial poll mode
	ADMR,	// address mode
	AUXMR,	// auxilliary mode
	ADR,	// address
	EOSR,	// end-of-string
};
// read registers
enum nec7210_read_regs
{
	DIR,	// data in
	ISR1,	// interrupt status 1
	ISR2,	// interrupt status 2
	SPSR,	// serial poll status
	ADSR,	// address status
	CPTR,	// command pass though
	ADR0,	// address 1
	ADR1,	// address 2
};

//bit definitions common to nec-7210 compatible registers

// ISR1: interrupt status register 1
enum isr1_bits
{
	HR_DI = ( 1 << 0 ),
	HR_DO = ( 1 << 1 ),
	HR_ERR = ( 1 << 2 ),
	HR_DEC = ( 1 << 3 ),
	HR_END = ( 1 << 4 ),
	HR_DET = ( 1 << 5 ),
	HR_APT = ( 1 << 6 ),
	HR_CPT = ( 1 << 7 ),
};

// IMR1: interrupt mask register 1
enum imr1_bits
{
	HR_DIIE = ( 1 << 0 ),
	HR_DOIE = ( 1 << 1 ),
	HR_ERRIE = ( 1 << 2 ),
	HR_DECIE = ( 1 << 3 ),
	HR_ENDIE = ( 1 << 4 ),
	HR_DETIE = ( 1 << 5 ),
	HR_APTIE = ( 1 << 6 ),
	HR_CPTIE = ( 1 << 7 ),
};

// ISR2, interrupt status register 2
enum isr2_bits
{
	HR_ADSC = ( 1 << 0 ),
	HR_REMC = ( 1 << 1 ),
	HR_LOKC = ( 1 << 2 ),
	HR_CO = ( 1 << 3 ),
	HR_REM = ( 1 << 4 ),
	HR_LOK = ( 1 << 5 ),
	HR_SRQI = ( 1 << 6 ),
	HR_INT = ( 1 << 7 ),
};

// IMR2, interrupt mask register 2
enum imr2_bits
{
	// all the bits in this register that enable interrupts
	IMR2_ENABLE_INTR_MASK = 0x4f,
	HR_ACIE = ( 1 << 0 ),
	HR_REMIE = ( 1 << 1 ),
	HR_LOKIE = ( 1 << 2 ),
	HR_COIE = ( 1 << 3 ),
	HR_DMAI = ( 1 << 4 ),
	HR_DMAO = ( 1 << 5 ),
	HR_SRQIE = ( 1 << 6 ),
};

// SPSR, serial poll status register
enum spsr_bits
{
	HR_PEND = ( 1 << 6 ),
};

// SPMR, serial poll mode register
enum spmr_bits
{
	HR_RSV = ( 1 << 6 ),
};

// ADSR, address status register
enum adsr_bits
{
	HR_MJMN = ( 1 << 0 ),
	HR_TA = ( 1 << 1 ),
	HR_LA = ( 1 << 2 ),
	HR_TPAS = ( 1 << 3 ),
	HR_LPAS = ( 1 << 4 ),
	HR_SPMS = ( 1 << 5 ),
	HR_NATN = ( 1 << 6 ),
	HR_CIC = ( 1 << 7 ),
};

// ADMR, address mode register
enum admr_bits
{
	HR_ADM0 = ( 1 << 0 ),
	HR_ADM1 = ( 1 << 1 ),
	HR_TRM0 = ( 1 << 4 ),
	HR_TRM1 = ( 1 << 5 ),
	HR_LON = ( 1 << 6 ),
	HR_TON = ( 1 << 7 ),
};

// ADR, bits used in address0, address1 and address0/1 registers
enum adr_bits
{
	ADDRESS_MASK = 0x1f,	/* mask to specify lower 5 bits */
	HR_DL = ( 1 << 5 ),
	HR_DT = ( 1 << 6 ),
	HR_ARS = ( 1 << 7 ),
};

// ADR1, address1 register
enum adr1_bits
{
	HR_EOI = ( 1 << 7 ),
};

// AUXMR, auxiliary mode register
enum auxmr_bits
{
	ICR = 0x20,
	PPR = 0x60,
	AUXRA = 0x80,
	AUXRB = 0xa0,
	AUXRE = 0xc0,
};

// auxra, auxiliary register A
enum auxra_bits
{
	HR_HANDSHAKE_MASK = 0x3,
	HR_HLDA = 0x1,
	HR_HLDE = 0x2,
	HR_LCM = 0x3,	/* auxra listen continuous */
	HR_REOS = 0x4,
	HR_XEOS = 0x8,
	HR_BIN = 0x10,
};

// auxrb, auxiliary register B
enum auxrb_bits
{
	HR_CPTE = ( 1 << 0 ),
	HR_SPEOI = ( 1 << 1 ),
	HR_TRI = ( 1 << 2 ),
	HR_INV = ( 1 << 3 ),
	HR_ISS = ( 1 << 4 ),
};

enum auxre_bits
{
	HR_DAC_HLD_DCAS = 0x1,	/* perform DAC holdoff on receiving clear */
	HR_DAC_HLD_DTAS = 0x2,	/* perform DAC holdoff on receiving trigger */
};

// parallel poll register
enum ppr_bits
{
	HR_PPS = ( 1 << 3 ),
	HR_PPU = ( 1 << 4 ),
};

/* 7210 Auxiliary Commands */
enum aux_cmds
{
	AUX_PON = 0x0,	/* Immediate Execute pon                  */
	AUX_CPPF = 0x1,	/* Clear Parallel Poll Flag               */
	AUX_CR = 0x2,	/* Chip Reset                             */
	AUX_FH = 0x3,	/* Finish Handshake                       */
	AUX_TRIG = 0x4,	/* Trigger                                */
	AUX_RTL = 0x5,	/* Return to local                        */
	AUX_SEOI = 0x6,	/* Send EOI                               */
	AUX_NVAL = 0x7,	/* Non-Valid Secondary Command or Address */
	AUX_SPPF = 0x9,	/* Set Parallel Poll Flag                 */
	AUX_VAL = 0xf,	/* Valid Secondary Command or Address     */
	AUX_GTS = 0x10,	/* Go To Standby                          */
	AUX_TCA = 0x11,	/* Take Control Asynchronously            */
	AUX_TCS = 0x12,	/* Take Control Synchronously             */
	AUX_LTN = 0x13,	/* Listen                                 */
	AUX_DSC = 0x14,	/* Disable System Control                 */
	AUX_CIFC = 0x16,	/* Clear IFC                              */
	AUX_CREN = 0x17,	/* Clear REN                              */
	AUX_TCSE = 0x1a,	/* Take Control Synchronously on End      */
	AUX_LTNC = 0x1b,	/* Listen in Continuous Mode              */
	AUX_LUN = 0x1c,	/* Local Unlisten                         */
	AUX_EPP = 0x1d,	/* Execute Parallel Poll                  */
	AUX_SIFC = 0x1e,	/* Set IFC                                */
	AUX_SREN = 0x1f,	/* Set REN                                */
};

#endif	//_NEC7210_H
