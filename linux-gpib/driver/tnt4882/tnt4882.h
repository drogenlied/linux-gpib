/***************************************************************************
                              tnt4882.h
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

#ifndef _TNT4882_H
#define _TNT4882_H

#include "nec7210.h"
#include "gpibP.h"
#include "mite.h"
#include <linux/init.h>
#include <linux/isapnp.h>
#include <linux/delay.h>

static const int ISAPNP_VENDOR_ID_NI = ISAPNP_VENDOR( 'N', 'I', 'C' );
static const int ISAPNP_ID_NI_ATGPIB_TNT = 0xc601;
enum
{
	PCI_DEVICE_ID_NI_GPIB = 0xc801,
	PCI_DEVICE_ID_NI_GPIB_PLUS = 0xc811,
};

typedef enum
{
	/* leave 0 unused to catch initialization bugs */
	TNT4882 = 1,
	NAT4882 = 2,
	NEC7210 = 3,
} ni_chipset_t;

// struct which defines private_data for tnt4882 devices
typedef struct
{
	nec7210_private_t nec7210_priv;
	struct mite_struct *mite;
	struct pci_dev *isapnp_dev;
	unsigned int irq;
	volatile int imr3_bits;
	ni_chipset_t chipset;
	void (*io_writeb)( unsigned int value, unsigned long address );
	void (*io_writew)( unsigned int value, unsigned long address );
	unsigned int (*io_readb)( unsigned long address );
	unsigned int (*io_readw)( unsigned long address );
} tnt4882_private_t;

// interfaces
extern gpib_interface_t ni_isa_interface;
extern gpib_interface_t ni_isa_accel_interface;
extern gpib_interface_t ni_pci_interface;
extern gpib_interface_t ni_pci_accel_interface;
extern gpib_interface_t ni_pcmcia_interface;
extern gpib_interface_t ni_pcmcia_accel_interface;

// interface functions
ssize_t tnt4882_read(gpib_board_t *board, uint8_t *buffer, size_t length,
	int *end);
ssize_t tnt4882_accel_read(gpib_board_t *board, uint8_t *buffer, size_t length,
	int *end);
ssize_t tnt4882_write(gpib_board_t *board, uint8_t *buffer, size_t length,
	int send_eoi);
ssize_t tnt4882_accel_write(gpib_board_t *board, uint8_t *buffer, size_t length,
	int send_eoi);
ssize_t tnt4882_command(gpib_board_t *board, uint8_t *buffer, size_t length);
int tnt4882_take_control(gpib_board_t *board, int synchronous);
int tnt4882_go_to_standby(gpib_board_t *board);
void tnt4882_request_system_control( gpib_board_t *board, int request_control );
void tnt4882_interface_clear(gpib_board_t *board, int assert);
void tnt4882_remote_enable(gpib_board_t *board, int enable);
void tnt4882_enable_eos(gpib_board_t *board, uint8_t eos_byte, int
 compare_8_bits);
void tnt4882_disable_eos(gpib_board_t *board);
unsigned int tnt4882_update_status( gpib_board_t *board, unsigned int clear_mask );
void tnt4882_primary_address(gpib_board_t *board, unsigned int address);
void tnt4882_secondary_address(gpib_board_t *board, unsigned int address, int
 enable);
int tnt4882_parallel_poll(gpib_board_t *board, uint8_t *result);
void tnt4882_parallel_poll_configure( gpib_board_t *board, uint8_t config );
void tnt4882_parallel_poll_response( gpib_board_t *board, int ist );
void tnt4882_serial_poll_response(gpib_board_t *board, uint8_t status);
uint8_t tnt4882_serial_poll_status( gpib_board_t *board );
int tnt4882_line_status( const gpib_board_t *board );
unsigned int tnt4882_t1_delay( gpib_board_t *board, unsigned int nano_sec );
void tnt4882_return_to_local( gpib_board_t *board );

// pcmcia init/cleanup
int __init init_ni_gpib_cs(void);
void __exit exit_ni_gpib_cs(void);

// interrupt service routines
void tnt4882_interrupt(int irq, void *arg, struct pt_regs *registerp);

// utility functions
int tnt4882_allocate_private(gpib_board_t *board);
void tnt4882_free_private(gpib_board_t *board);
void tnt4882_init( tnt4882_private_t *tnt_priv, const gpib_board_t *board,
	ni_chipset_t chipset );

// register offset for nec7210 compatible registers
static const int atgpib_reg_offset = 2;

// number of ioports used
static const int atgpib_iosize = 32;

// tnt4882 register offsets
enum
{
	ACCWR = 0x5,
	// offset of auxilliary command register in 9914 mode
	AUXCR = 0x6,
	INTRT = 0x7,
	// register number for auxilliary command register when swap bit is set (9914 mode)
	SWAPPED_AUXCR = 0xa,
	HSSEL = 0xd,	// handshake select register
	CNT2 = 0x9,
	CNT3 = 0xb,
	CFG = 0x10,
	SASR = 0x1b,
	IMR0 = 0x1d,
	IMR3 = 0x12,
	CNT0 = 0x14,
	CNT1 = 0x16,
	KEYREG = 0x17,	// key control register (7210 mode only)
	CSR = KEYREG,
	FIFOB = 0x18,
	FIFOA = 0x19,
	CCR = 0x1a,	// carry cycle register
	CMDR = 0x1c,	// command register
	TIMER = 0x1e,	// timer register

	STS1 = 0x10,		/* T488 Status Register 1 */
	STS2 = 0x1c,	        /* T488 Status Register 2 */
	ISR0 = IMR0,
	ISR3 = 0x1a,		/* T488 Interrupt Status Register 3 */
	BCR = 0x1f,		/* bus control/status register */
	BSR = BCR,
};
static const int tnt_pagein_offset = 0x11;

/*============================================================*/

/* TURBO-488 registers bit definitions */

enum bus_control_status_bits
{
	BCSR_REN_BIT = 0x1,
	BCSR_IFC_BIT = 0x2,
	BCSR_SRQ_BIT = 0x4,
	BCSR_EOI_BIT = 0x8,
	BCSR_NRFD_BIT = 0x10,
	BCSR_NDAC_BIT = 0x20,
	BCSR_DAV_BIT = 0x40,
	BCSR_ATN_BIT = 0x80,
};

/* CFG -- Configuration Register (write only) */
enum cfg_bits
{
	TNT_TLCHE = ( 1 << 6 ),	/* halt transfer on imr0, imr1, or imr2 interrupt */
	TNT_IN = ( 1 << 5 ),	/* transfer is GPIB read                 */
	TNT_A_B = ( 1 << 4 ),	/* order to use fifos 1=fifa A first(big endian), 0=fifo b first(little endian) */
	TNT_CCEN = ( 1 << 3 ),	/* enable carry cycle                 */
	TNT_TMOE = ( 1 << 2 ),	/* enable CPU bus time limit          */
	TNT_TIM_BYTN = ( 1 << 1 ),	/* tmot reg is: 1=125ns clocks, 0=num bytes */
	TNT_B_16BIT = ( 1 << 0 ),	/* 1=FIFO is 16-bit register, 0=8-bit */
};

/* CMDR -- Command Register */
enum cmdr_bits
{
	CLRSC = 0x2,	/* clear the SC bit */
	SETSC = 0x3,	/* set the SC bit */
	GO = 0x4,	/* start fifos */
	STOP = 0x8,	/* stop fifos */
	RESET_FIFO = 0x10,	/* reset the FIFOs 		*/
	SOFT_RESET = 0x22,	/* issue a software reset 	*/
};

/* HSSEL -- handshake select register (write only) */
enum hssel_bits
{
	NODMA = 0x10,
};

/* IMR0 -- Interrupt Mode Register 0 */
enum imr0_bits
{
	TNT_SYNCIE_BIT = 0x1, /* handshake sync */
	TNT_TOIE_BIT = 0x2, /* timeout */
	TNT_ATNIE_BIT = 0x4, /* ATN interrupt */
	TNT_IFCIE_BIT = 0x8,	/* interface clear interrupt */
	TNT_BTO_BIT = 0x10, /* byte timeout */
	TNT_NLEN_BIT = 0x20,	/* treat new line as EOS char */
	TNT_STBOIE_BIT = 0x40,	/* status byte out  */
	TNT_IMR0_ALWAYS_BITS = 0x80,	/* always set this bit on write */
};

/* ISR0 -- Interrupt Status Register 0 */
enum isr0_bits
{
	TNT_SYNC_BIT = 0x1, /* handshake sync */
	TNT_TO_BIT = 0x2, /* timeout */
	TNT_ATNI_BIT = 0x4, /* ATN interrupt */
	TNT_IFCI_BIT = 0x8,	/* interface clear interrupt */
	TNT_EOS_BIT = 0x10, /* end of string */
	TNT_NL_BIT = 0x20,	/* new line receive */
	TNT_STBO_BIT = 0x40,	/* status byte out  */
	TNT_NBA_BIT = 0x80,	/* new byte available */
};

/* ISR3 -- Interrupt Status Register 3 (read only) */
enum isr3_bits
{
	HR_DONE = ( 1 << 0 ),	/* transfer done */
	HR_TLCI = ( 1 << 1 ),	/* TLC interrupt asserted */
	HR_NEF = ( 1 << 2 ),	/* NOT empty fifo */
	HR_NFF = ( 1 << 3 ),	/* NOT full fifo */
	HR_STOP = ( 1 << 4 ),	/* fifo empty or STOP command issued */
	HR_SRQI_CIC = ( 1 << 5 ),	/* SRQ asserted and we are CIC */
	HR_INTR = ( 1 << 7 ),	/* 1=board is interrupting */
};

enum keyreg_bits
{
	MSTD = 0x20,	// enable 350ns T1 delay
};

/* STS1 -- Status Register 1 (read only) */
enum sts1_bits
{
	S_DONE = 0x80,	/* DMA done                           */
	S_SC = 0x40,	/* is system contoller                */
	S_IN = 0x20,	/* DMA in (to memory)                 */
	S_DRQ = 0x10,	/* DRQ line (for diagnostics)         */
	S_STOP = 0x08,	/* DMA stopped                        */
	S_NDAV = 0x04,	/* inverse of DAV                     */
	S_HALT = 0x02,	/* status of transfer machine         */
	S_GSYNC = 0x01,	/* indicates if GPIB is in sync w I/O */
};

/* STS2 -- Status Register 2 */
enum sts2_bits
{
	AFFN = ( 1 << 3 ), 	/* "A full FIFO NOT"  (0=FIFO full)  */
	AEFN = ( 1 << 2 ),	/* "A empty FIFO NOT" (0=FIFO empty) */
	BFFN = ( 1 << 1 ),	/* "B full FIFO NOT"  (0=FIFO full)  */
	BEFN = ( 1 << 0 ),	/* "B empty FIFO NOT" (0=FIFO empty) */
};

// Auxilliary commands
enum tnt4882_aux_cmds
{
	AUX_9914 = 0x15,	// switch to 9914 mode
	AUX_PAGEIN = 0x50,	/* page in alternate registers */
	AUX_HLDI = 0x51,	// rfd holdoff immediately
	AUX_7210 = 0x99,	// switch to 7210 mode
};

enum tnt4882_aux_regs
{
	AUXRI = 0xe0,
};

enum auxi_bits
{
	SISB = 0x1,	// static interrupt bits (don't clear isr1, isr2 on read )
	PP2 = 0x4,	// ignore remote parallel poll configuration
	USTD = 0x8,	// ultra short ( 1100 nanosec ) T1 delay
};

/* paged io */
static inline unsigned int tnt_paged_readb( tnt4882_private_t *priv, unsigned long offset )
{
	unsigned long address = priv->nec7210_priv.iobase + offset;

	write_byte( &priv->nec7210_priv, AUX_PAGEIN, AUXMR );
	return priv->io_readb( address );
}
static inline void tnt_paged_writeb(tnt4882_private_t *priv, unsigned int value, unsigned long offset )
{
	unsigned long address = priv->nec7210_priv.iobase + offset;

	write_byte( &priv->nec7210_priv, AUX_PAGEIN, AUXMR );
	priv->io_writeb( value, address );
}

/* readb/writeb wrappers */
static inline unsigned int tnt_readb( tnt4882_private_t *priv, unsigned long offset )
{
	unsigned long address = priv->nec7210_priv.iobase + offset;

	switch( offset )
	{
	case CSR:
	case SASR:
	case ISR0:
	case BSR:
		switch( priv->chipset )
		{
		case TNT4882:
			return priv->io_readb( address );
			break;
		case NAT4882:
			return tnt_paged_readb( priv, offset - tnt_pagein_offset );
			break;
		case NEC7210:
			return 0;
			break;
		default:
			printk( "tnt4882: bug! unsupported ni_chipset\n" );
			return 0;
			break;
		}
		break;
	default:
		break;
	}
	return priv->io_readb( address );
}

static inline void tnt_writeb( tnt4882_private_t *priv, unsigned int value, unsigned long offset)
{
	unsigned long address = priv->nec7210_priv.iobase + offset;

	switch( offset )
	{
	case KEYREG:
	case IMR0:
	case BCR:
		switch( priv->chipset )
		{
		case TNT4882:
			priv->io_writeb( value, address );
			break;
		case NAT4882:
			tnt_paged_writeb( priv, value, offset - tnt_pagein_offset );
			break;
		case NEC7210:
			break;
		default:
			printk( "tnt4882: bug! unsupported ni_chipset\n" );
			break;
		}
		break;
	case HSSEL:
		switch( priv->chipset )
		{
		case TNT4882:
			priv->io_writeb( value, address );
			break;
		default:
			break;
		}
		break;
	default:
		priv->io_writeb( value, address );
		break;
	}
}
#endif	// _TNT4882_H
