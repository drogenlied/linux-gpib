/***************************************************************************
                              nec7210/cb7210.h
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

#ifndef _CB7210_H
#define _CB7210_H

#include <linux/config.h>
#include "nec7210.h"
#include "gpibP.h"
#include "amccs5933.h"

#define PCI_DEVICE_ID_CBOARDS_PCI_GPIB 0x6

// struct which defines private_data for cb7210 boards
typedef struct
{
	nec7210_private_t nec7210_priv;
	struct pci_dev *pci_device;
	// base address of amccs5933 pci chip
	unsigned long amcc_iobase;
	unsigned int irq;
	volatile uint8_t hs_mode_bits;
	volatile unsigned out_fifo_half_empty : 1;
	volatile unsigned in_fifo_half_full : 1;
} cb7210_private_t;

// interfaces
extern gpib_interface_t cb_pci_interface;
extern gpib_interface_t cb_isa_interface;
extern gpib_interface_t cb_pcmcia_interface;
extern gpib_interface_t cb_pcmcia_accel_interface;

// interrupt service routines
void cb_pci_interrupt(int irq, void *arg, struct pt_regs *registerp);
void cb7210_interrupt(int irq, void *arg, struct pt_regs *registerp);

// interface functions
ssize_t cb7210_read( gpib_board_t *board, uint8_t *buffer, size_t length,
	int *end );
ssize_t cb7210_accel_read( gpib_board_t *board, uint8_t *buffer, size_t length,
	int *end );
ssize_t cb7210_write( gpib_board_t *board, uint8_t *buffer, size_t length,
	int send_eoi );
ssize_t cb7210_accel_write( gpib_board_t *board, uint8_t *buffer, size_t length,
	int send_eoi );
ssize_t cb7210_command(gpib_board_t *board, uint8_t *buffer, size_t length);
int cb7210_take_control(gpib_board_t *board, int synchronous);
int cb7210_go_to_standby(gpib_board_t *board);
void cb7210_request_system_control( gpib_board_t *board, int request_control );
void cb7210_interface_clear(gpib_board_t *board, int assert);
void cb7210_remote_enable(gpib_board_t *board, int enable);
void cb7210_enable_eos(gpib_board_t *board, uint8_t eos_byte,
	int compare_8_bits);
void cb7210_disable_eos(gpib_board_t *board);
unsigned int cb7210_update_status(gpib_board_t *board);
void cb7210_primary_address(gpib_board_t *board, unsigned int address);
void cb7210_secondary_address(gpib_board_t *board, unsigned int address,
	int enable);
int cb7210_parallel_poll(gpib_board_t *board, uint8_t *result);
void cb7210_serial_poll_response(gpib_board_t *board, uint8_t status);
uint8_t cb7210_serial_poll_status( gpib_board_t *board );
void cb7210_parallel_poll_response(gpib_board_t *board, uint8_t configuration);
int cb7210_line_status( const gpib_board_t *board );

// utility functions
void cb7210_generic_detach(gpib_board_t *board);
int cb7210_generic_attach(gpib_board_t *board);
void cb7210_init( cb7210_private_t *priv, const gpib_board_t *board, int accel );

// pcmcia init/cleanup
int cb_pcmcia_init_module(void);
void cb_pcmcia_cleanup_module(void);

// pci-gpib register offset
static const int cb7210_reg_offset = 1;

// uses 10 ioports
static const int cb7210_iosize = 10;

// fifo size in bytes
static const int cb7210_fifo_size = 2048;

// cb7210 specific registers and bits
enum cb7210_regs
{
	BUS_STATUS = 0x7,
};
enum cb7210_page_in
{
	BUS_STATUS_PAGE = 1,
};

static inline int page_in_bits( unsigned int page )
{
	return 0x50 | (page & 0xf);
}

enum hs_regs
{
	//write registers
	HS_MODE = 0x8,	/* HS_MODE register */
	HS_INT_LEVEL = 0x9,	/* HS_INT_LEVEL register */
	//read registers
	HS_STATUS = 0x8,	/* HS_STATUS register */
};

enum bus_status_bits
{
	BSR_ATN_BIT = 0x1,
	BSR_EOI_BIT = 0x2,
	BSR_SRQ_BIT = 0x4,
	BSR_IFC_BIT = 0x8,
	BSR_REN_BIT = 0x10,
	BSR_DAV_BIT = 0x20,
	BSR_NRFD_BIT = 0x40,
	BSR_NDAC_BIT = 0x80,
};

/* CBI 488.2 HS control */

/* when both bit 0 and 1 are set, it
 *   1 clears the transmit state machine to an initial condition
 *   2 clears any residual interrupts left latched on cbi488.2
 *   3 resets all control bits in HS_MODE to zero
 *   4 enables TX empty interrupts
 * when both bit 0 and 1 are zero, then the high speed mode is disabled
 */
enum hs_mode_bits
{
	HS_ENABLE_MASK = 0x3,
	HS_TX_ENABLE = ( 1 << 0 ),
	HS_RX_ENABLE = ( 1 << 1 ),
	HS_HF_INT_EN = ( 1 << 3 ),
	HS_CLR_SRQ_INT = ( 1 << 4 ),
	HS_CLR_EOI_EMPTY_INT = ( 1 << 5 ),
	HS_CLR_HF_INT = ( 1 << 6 ),
	HS_SYS_CONTROL = ( 1 << 7 ),
};

/* CBI 488.2 status */
enum hs_status_bits
{
	HS_FIFO_FULL = ( 1 << 0 ),
	HS_HALF_FULL = ( 1 << 1 ),
	HS_SRQ_INT = ( 1 << 2 ),
	HS_EOI_INT = ( 1 << 3 ),
	HS_TX_MSB_EMPTY = ( 1 << 4 ),
	HS_RX_MSB_NOT_EMPTY = ( 1 << 5 ),
	HS_TX_LSB_EMPTY = ( 1 << 6 ),
	HS_RX_LSB_NOT_EMPTY = ( 1 << 7 ),
};

/* CBI488.2 hs_int_level register */
enum hs_int_level_bits
{
	HS_RESET7210 = ( 1 << 7 ),
};
/*
AUX_HISPEED     0x41
AUX_LOSPEED     0x40
*/

#endif	// _CB7210_H
