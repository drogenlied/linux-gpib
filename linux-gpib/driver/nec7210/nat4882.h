/***************************************************************************
                              nec7210/nat4882.h
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

#ifndef _NAT4882_H
#define _NAT4882_H

// struct which defines private_data for nat4882 devices
typedef struct
{
	nec7210_private_t nec7210_priv;
	struct pci_dev *pci_device;
	unsigned int irq;
} nat4882_private_t;

// interfaces
extern gpib_interface_t ni_isa_interface;

// interface functions
ssize_t nat4882_read(gpib_device_t *device, uint8_t *buffer, size_t length, int
 *end);
ssize_t nat4882_write(gpib_device_t *device, uint8_t *buffer, size_t length, int
 send_eoi);
ssize_t nat4882_command(gpib_device_t *device, uint8_t *buffer, size_t length);
int nat4882_take_control(gpib_device_t *device, int synchronous);
int nat4882_go_to_standby(gpib_device_t *device);
void nat4882_interface_clear(gpib_device_t *device, int assert);
void nat4882_remote_enable(gpib_device_t *device, int enable);
void nat4882_enable_eos(gpib_device_t *device, uint8_t eos_byte, int
 compare_8_bits);
void nat4882_disable_eos(gpib_device_t *device);
unsigned int nat4882_update_status(gpib_device_t *device);
void nat4882_primary_address(gpib_device_t *device, unsigned int address);
void nat4882_secondary_address(gpib_device_t *device, unsigned int address, int
 enable);
int nat4882_parallel_poll(gpib_device_t *device, uint8_t *result);
int nat4882_serial_poll_response(gpib_device_t *device, uint8_t status);

// interrupt service routines
void nat4882_interrupt(int irq, void *arg, struct pt_regs *registerp);

// utility functions
int nat4882_allocate_private(gpib_device_t *device);
void nat4882_free_private(gpib_device_t *device);

// register offset for nec7210 compatible registers
static const int atgpib_reg_offset = 2;

// number of ioports used
static const int atgpib_iosize = 32;

// nat4882 specific registers and bits
#define GPIBC	0x1
#define GPIBD	0x3
#define KEYPRT	0x5
#define INTRT 0x7
/* TURBO488...                          */
#define CFG	0x10
#define IMR3	0x12
#define CNTL	0x14
#define CNTH	0x16
#define FIFO	0x18	// fifo can be single 16 bit register or two 8 bit
#define FIFOB	0x18
#define FIFOA	0x19
#define CCRG	0x1a	// carry cycle register
#define CMDR	0x1c	// command register
#define TIMER	0x1e	// timer register

#define STS1	0x10		/* T488 Status Register 1 */
#define STS2	0x1c	        /* T488 Status Register 2 */
#define ISR3	0x1a		/* T488 Interrupt Status Register 3 */
#define DMA_EN	0x5		/* DMA Enable Register (Key Port) */



#endif	// _NAT4882_H
