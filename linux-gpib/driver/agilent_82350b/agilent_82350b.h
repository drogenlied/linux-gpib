/***************************************************************************
                              agilent_82350b/agilent_82350b.h
                             -------------------

    copyright            : (C) 2002, 2004 by Frank Mori Hess
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

#ifndef _AGILENT_82350B_H
#define _AGILENT_82350B_H

#include "tms9914.h"
#include "gpibP.h"


enum pci_vendor_ids
{
	PCI_VENDOR_ID_AGILENT = 0x0,
};

enum pci_device_ids
{
	PCI_DEVICE_ID_82350B = 0x0,
};

// struct which defines private_data for board
typedef struct
{
	tms9914_private_t tms9914_priv;
} agilent_82350b_private_t;

// interfaces
extern gpib_interface_t agilent_82350b_interface;

// interface functions
ssize_t agilent_82350b_read( gpib_board_t *board, uint8_t *buffer, size_t length, int *end, int *nbytes);
ssize_t agilent_82350b_write( gpib_board_t *board, uint8_t *buffer, size_t length, int send_eoi );
ssize_t agilent_82350b_command( gpib_board_t *board, uint8_t *buffer, size_t length );
int agilent_82350b_take_control( gpib_board_t *board, int synchronous );
int agilent_82350b_go_to_standby( gpib_board_t *board );
void agilent_82350b_request_system_control( gpib_board_t *board, int request_control );
void agilent_82350b_interface_clear( gpib_board_t *board, int assert );
void agilent_82350b_remote_enable( gpib_board_t *board, int enable );
void agilent_82350b_enable_eos( gpib_board_t *board, uint8_t eos_byte, int
 compare_8_bits );
void agilent_82350b_disable_eos( gpib_board_t *board );
unsigned int agilent_82350b_update_status( gpib_board_t *board, unsigned int clear_mask );
void agilent_82350b_primary_address( gpib_board_t *board, unsigned int address );
void agilent_82350b_secondary_address( gpib_board_t *board, unsigned int address, int
 enable );
int agilent_82350b_parallel_poll( gpib_board_t *board, uint8_t *result );
void agilent_82350b_parallel_poll_configure( gpib_board_t *board, uint8_t config );
void agilent_82350b_parallel_poll_response( gpib_board_t *board, int ist );
void agilent_82350b_serial_poll_response( gpib_board_t *board, uint8_t status );
void agilent_82350b_return_to_local( gpib_board_t *board );

// interrupt service routines
irqreturn_t agilent_82350b_interrupt(int irq, void *arg, struct pt_regs *registerp);

// utility functions
int agilent_82350b_allocate_private(gpib_board_t *board);
void agilent_82350b_free_private(gpib_board_t *board);

// register offset for tms9914 compatible registers
static const int atgpib_reg_offset = 2;

#endif	// _AGILENT_82350B_H
