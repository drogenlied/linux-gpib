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

typedef struct
{
	nec7210_private_t nec7210_priv;
	struct pci_dev *pci_device;
	// base address for plx pci chip
	unsigned long plx_iobase;
	unsigned int irq;
} ines_private_t;

// interfaces
extern gpib_interface_t ines_pci_interface;
extern gpib_interface_t ines_pcmcia_interface;

// interface functions
ssize_t ines_read(gpib_device_t *device, uint8_t *buffer, size_t length, int *end);
ssize_t ines_write(gpib_device_t *device, uint8_t *buffer, size_t length, int send_eoi);
ssize_t ines_command(gpib_device_t *device, uint8_t *buffer, size_t length);
int ines_take_control(gpib_device_t *device, int synchronous);
int ines_go_to_standby(gpib_device_t *device);
void ines_interface_clear(gpib_device_t *device, int assert);
void ines_remote_enable(gpib_device_t *device, int enable);
void ines_enable_eos(gpib_device_t *device, uint8_t eos_byte, int compare_8_bits);
void ines_disable_eos(gpib_device_t *device);
unsigned int ines_update_status(gpib_device_t *device);
void ines_primary_address(gpib_device_t *device, unsigned int address);
void ines_secondary_address(gpib_device_t *device, unsigned int address, int enable);
int ines_parallel_poll(gpib_device_t *device, uint8_t *result);
int ines_serial_poll_response(gpib_device_t *device, uint8_t status);

// interrupt service routines
void ines_pci_interrupt(int irq, void *arg, struct pt_regs *registerp);
void ines_interrupt(int irq, void *arg, struct pt_regs *registerp);

// utility functions
int ines_allocate_private(gpib_device_t *device);
void ines_free_private(gpib_device_t *device);

// pcmcia init/cleanup
int ines_pcmcia_init_module(void);
void ines_pcmcia_cleanup_module(void);

// offset between consecutive nec7210 registers
static const int ines_reg_offset = 1;

// plx pci chip registers and bits
enum
{
	PLX_INTCSR_REG = 0x4c,
};

enum
{
	INTCSR_ENABLE_INTR = 0x63,
	INTCSR_DISABLE_INTR = 0x62,
};

#endif _INES_GPIB_H
