/***************************************************************************
                              nec7210/board.h
                             -------------------

    begin                : Dec 2001
    copyright            : (C) 2001, 2002 by Frank Mori Hess
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


#ifndef _GPIB_PCIIA_BOARD_H
#define _GPIB_PCIIA_BOARD_H

#include <gpibP.h>
#include <asm/io.h>
#include <gpib_buffer.h>

#include "nec7210.h"
#include "pc2.h"
#include "cb7210.h"

extern ssize_t nec7210_read(gpib_driver_t *driver, uint8_t *buffer, size_t length, int *end);
extern ssize_t nec7210_write(gpib_driver_t *driver, uint8_t *buffer, size_t length, int send_eoi);
extern ssize_t nec7210_command(gpib_driver_t *driver, uint8_t *buffer, size_t length);
extern int nec7210_take_control(gpib_driver_t *driver, int syncronous);
extern int nec7210_go_to_standby(gpib_driver_t *driver);
extern void nec7210_interface_clear(gpib_driver_t *driver, int assert);
extern void nec7210_remote_enable(gpib_driver_t *driver, int enable);
extern void nec7210_enable_eos(gpib_driver_t *driver, uint8_t eos_bytes, int compare_8_bits);
extern void nec7210_disable_eos(gpib_driver_t *driver);
extern unsigned int nec7210_update_status(gpib_driver_t *driver);
extern void nec7210_primary_address(gpib_driver_t *driver, unsigned int address);
extern void nec7210_secondary_address(gpib_driver_t *driver, unsigned int address, int enable);
extern int nec7210_parallel_poll(gpib_driver_t *driver, uint8_t *result);
extern int nec7210_serial_poll_response(gpib_driver_t *driver, uint8_t status);

extern unsigned long ibbase;	/* base addr of GPIB interface registers  */
extern unsigned long remapped_ibbase;	// ioremapped memory io address
extern unsigned int ibirq;	/* interrupt request line for GPIB (1-7)  */
extern unsigned int ibdma ;      /* DMA channel                            */
extern struct pci_dev *pci_dev_ptr;	// pci_dev for plug and play boards

extern gpib_buffer_t *read_buffer, *write_buffer;

// interrupt service routine
void nec7210_interrupt(int irq, void *arg, struct pt_regs *registerp);

#endif	//_GPIB_PCIIA_BOARD_H

