/***************************************************************************
                          cec/cec.h  -  description
                             -------------------
  Header for cec GPIB boards

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

#ifndef _CEC_GPIB_H
#define _CEC_GPIB_H

#include <nec7210.h>
#include <gpibP.h>
#include <plx9050.h>
#include <linux/config.h>

typedef struct
{
	nec7210_private_t nec7210_priv;
	struct pci_dev *pci_device;
	// base address for plx9052 pci chip
	unsigned long plx_iobase;
	unsigned int irq;
} cec_private_t;

// interfaces
extern gpib_interface_t cec_pci_interface;
extern gpib_interface_t cec_pcmcia_interface;

// interface functions
ssize_t cec_read(gpib_device_t *device, uint8_t *buffer, size_t length, int *end);
ssize_t cec_write(gpib_device_t *device, uint8_t *buffer, size_t length, int send_eoi);
ssize_t cec_command(gpib_device_t *device, uint8_t *buffer, size_t length);
int cec_take_control(gpib_device_t *device, int synchronous);
int cec_go_to_standby(gpib_device_t *device);
void cec_interface_clear(gpib_device_t *device, int assert);
void cec_remote_enable(gpib_device_t *device, int enable);
void cec_enable_eos(gpib_device_t *device, uint8_t eos_byte, int compare_8_bits);
void cec_disable_eos(gpib_device_t *device);
unsigned int cec_update_status(gpib_device_t *device);
void cec_primary_address(gpib_device_t *device, unsigned int address);
void cec_secondary_address(gpib_device_t *device, unsigned int address, int enable);
int cec_parallel_poll(gpib_device_t *device, uint8_t *result);
int cec_serial_poll_response(gpib_device_t *device, uint8_t status);

// interrupt service routines
void cec_interrupt(int irq, void *arg, struct pt_regs *registerp);

// utility functions
void cec_free_private(gpib_device_t *device);
int cec_generic_attach(gpib_device_t *device);
void cec_init(cec_private_t *priv);

// offset between consecutive nec7210 registers
static const int cec_reg_offset = 1;

#endif	// _CEC_GPIB_H
