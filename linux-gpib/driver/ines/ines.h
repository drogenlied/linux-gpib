/***************************************************************************
                          nec7210/ines.h  -  description
                             -------------------
  Header for ines GPIB boards

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

#ifndef _INES_GPIB_H
#define _INES_GPIB_H

#include <nec7210.h>
#include <gpibP.h>
#include <plx9050.h>
#include <amcc5920.h>
#include <linux/config.h>

typedef struct
{
	nec7210_private_t nec7210_priv;
	struct pci_dev *pci_device;
	// base address for plx9052 pci chip
	unsigned long plx_iobase;
	// base address for amcc5920 pci chip
	unsigned long amcc_iobase;
	unsigned int irq;
} ines_private_t;

// interfaces
extern gpib_interface_t ines_pci_interface;
extern gpib_interface_t ines_pcmcia_interface;

// interface functions
ssize_t ines_read(gpib_board_t *board, uint8_t *buffer, size_t length, int *end);
ssize_t ines_write(gpib_board_t *board, uint8_t *buffer, size_t length, int send_eoi);
ssize_t ines_command(gpib_board_t *board, uint8_t *buffer, size_t length);
int ines_take_control(gpib_board_t *board, int synchronous);
int ines_go_to_standby(gpib_board_t *board);
void ines_interface_clear(gpib_board_t *board, int assert);
void ines_remote_enable(gpib_board_t *board, int enable);
void ines_enable_eos(gpib_board_t *board, uint8_t eos_byte, int compare_8_bits);
void ines_disable_eos(gpib_board_t *board);
unsigned int ines_update_status(gpib_board_t *board);
void ines_primary_address(gpib_board_t *board, unsigned int address);
void ines_secondary_address(gpib_board_t *board, unsigned int address, int enable);
int ines_parallel_poll(gpib_board_t *board, uint8_t *result);
void ines_parallel_poll_response( gpib_board_t *board, uint8_t config );
void ines_serial_poll_response(gpib_board_t *board, uint8_t status);
uint8_t ines_serial_poll_status( gpib_board_t *board );
int ines_line_status( const gpib_board_t *board );

// interrupt service routines
void ines_interrupt(int irq, void *arg, struct pt_regs *registerp);

// utility functions
void ines_free_private(gpib_board_t *board);
int ines_generic_attach(gpib_board_t *board);
void ines_init( ines_private_t *priv, const gpib_board_t *board );

// pcmcia init/cleanup
int ines_pcmcia_init_module(void);
void ines_pcmcia_cleanup_module(void);

// offset between consecutive nec7210 registers
static const int ines_reg_offset = 1;

enum ines_regs
{
	BUS_CONTROL_MONITOR = 0x13,
};

enum bus_control_monitor_bits
{
	BCM_DAV_BIT = 0x1,
	BCM_NRFD_BIT = 0x2,
	BCM_NDAC_BIT = 0x4,
	BCM_IFC_BIT = 0x8,
	BCM_ATN_BIT = 0x10,
	BCM_SRQ_BIT = 0x20,
	BCM_REN_BIT = 0x40,
	BCM_EOI_BIT = 0x80,
};

#endif	// _INES_GPIB_H
