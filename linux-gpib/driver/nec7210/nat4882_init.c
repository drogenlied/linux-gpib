/***************************************************************************
                          nec7210/nat4882_init.c  -  description
                             -------------------
 board specific initialization stuff for National Instruments boards
 using nat4882 or compatible chips (at-gpib, etc).

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

#include "board.h"
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <asm/dma.h>
#include <gpib_buffer.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/string.h>

int ni_isa_attach(gpib_device_t *device);

void ni_isa_detach(gpib_device_t *device);

// wrappers for interface functions
ssize_t nat4882_read(gpib_device_t *device, uint8_t *buffer, size_t length, int *end)
{
	nat4882_private_t *priv = device->private_data;
	return nec7210_read(device, &priv->nec7210_priv, buffer, length, end);
}
ssize_t nat4882_write(gpib_device_t *device, uint8_t *buffer, size_t length, int send_eoi)
{
	nat4882_private_t *priv = device->private_data;
	return nec7210_write(device, &priv->nec7210_priv, buffer, length, send_eoi);
}
ssize_t nat4882_command(gpib_device_t *device, uint8_t *buffer, size_t length)
{
	nat4882_private_t *priv = device->private_data;
	return nec7210_command(device, &priv->nec7210_priv, buffer, length);
}
int nat4882_take_control(gpib_device_t *device, int synchronous)
{
	nat4882_private_t *priv = device->private_data;
	return nec7210_take_control(device, &priv->nec7210_priv, synchronous);
}
int nat4882_go_to_standby(gpib_device_t *device)
{
	nat4882_private_t *priv = device->private_data;
	return nec7210_go_to_standby(device, &priv->nec7210_priv);
}
void nat4882_interface_clear(gpib_device_t *device, int assert)
{
	nat4882_private_t *priv = device->private_data;
	nec7210_interface_clear(device, &priv->nec7210_priv, assert);
}
void nat4882_remote_enable(gpib_device_t *device, int enable)
{
	nat4882_private_t *priv = device->private_data;
	nec7210_remote_enable(device, &priv->nec7210_priv, enable);
}
void nat4882_enable_eos(gpib_device_t *device, uint8_t eos_byte, int compare_8_bits)
{
	nat4882_private_t *priv = device->private_data;
	nec7210_enable_eos(device, &priv->nec7210_priv, eos_byte, compare_8_bits);
}
void nat4882_disable_eos(gpib_device_t *device)
{
	nat4882_private_t *priv = device->private_data;
	nec7210_disable_eos(device, &priv->nec7210_priv);
}
unsigned int nat4882_update_status(gpib_device_t *device)
{
	nat4882_private_t *priv = device->private_data;
	return nec7210_update_status(device, &priv->nec7210_priv);
}
void nat4882_primary_address(gpib_device_t *device, unsigned int address)
{
	nat4882_private_t *priv = device->private_data;
	nec7210_primary_address(device, &priv->nec7210_priv, address);
}
void nat4882_secondary_address(gpib_device_t *device, unsigned int address, int enable)
{
	nat4882_private_t *priv = device->private_data;
	nec7210_secondary_address(device, &priv->nec7210_priv, address, enable);
}
int nat4882_parallel_poll(gpib_device_t *device, uint8_t *result)
{
	nat4882_private_t *priv = device->private_data;
	return nec7210_parallel_poll(device, &priv->nec7210_priv, result);
}
int nat4882_serial_poll_response(gpib_device_t *device, uint8_t status)
{
	nat4882_private_t *priv = device->private_data;
	return nec7210_serial_poll_response(device, &priv->nec7210_priv, status);
}

gpib_interface_t ni_isa_interface =
{
	name: "atgpib",
	attach: ni_isa_attach,
	detach: ni_isa_detach,
	read: nat4882_read,
	write: nat4882_write,
	command: nat4882_command,
	take_control: nat4882_take_control,
	go_to_standby: nat4882_go_to_standby,
	interface_clear: nat4882_interface_clear,
	remote_enable: nat4882_remote_enable,
	enable_eos: nat4882_enable_eos,
	disable_eos: nat4882_disable_eos,
	parallel_poll: nat4882_parallel_poll,
	line_status: NULL,	//XXX
	update_status: nat4882_update_status,
	primary_address: nat4882_primary_address,
	secondary_address: nat4882_secondary_address,
	serial_poll_response: nat4882_serial_poll_response,
};

int nat4882_allocate_private(gpib_device_t *device)
{
	device->private_data = kmalloc(sizeof(nat4882_private_t), GFP_KERNEL);
	if(device->private_data == NULL)
		return -1;
	memset(device->private_data, 0, sizeof(nat4882_private_t));
	return 0;
}

void nat4882_free_private(gpib_device_t *device)
{
	if(device->private_data)
	{
		kfree(device->private_data);
		device->private_data = NULL;
	}
}

int ni_isa_attach(gpib_device_t *device)
{
	nat4882_private_t *nat_priv;
	nec7210_private_t *nec_priv;
	int isr_flags = 0;

	device->status = 0;

	if(nat4882_allocate_private(device))
		return -ENOMEM;
	nat_priv = device->private_data;
	nec_priv = &nat_priv->nec7210_priv;
	nec_priv->read_byte = ioport_read_byte;
	nec_priv->write_byte = ioport_write_byte;
	nec_priv->offset = atgpib_reg_offset;

	// allocate ioports
	if(check_region(device->ibbase, atgpib_iosize) < 0)
	{
		printk("gpib: ioports are already in use");
		return -1;
	}
	request_region(device->ibbase, atgpib_iosize, "atgpib");
	nec_priv->iobase = device->ibbase;

	// get irq
	if(request_irq(device->ibirq, nat4882_interrupt, isr_flags, "atgpib", device))
	{
		printk("gpib: can't request IRQ %d\n", device->ibirq);
		return -1;
	}
	nat_priv->irq = device->ibirq;

	/* NAT 4882 reset */
	udelay(1);
	outb(SFTRST, nec_priv->iobase + CMDR);	/* Turbo488 software reset */

	nec7210_board_reset(nec_priv);

	// enable interrupt
	outb(1, nec_priv->iobase + INTRT);

	// XXX set clock register for 20MHz? driving frequency
	write_byte(nec_priv, ICR | 8, AUXMR);

	// enable nec7210 interrupts
	nec_priv->imr1_bits = HR_ERRIE | HR_DECIE | HR_ENDIE |
		HR_DETIE | HR_APTIE | HR_CPTIE;
	nec_priv->imr2_bits = IMR2_ENABLE_INTR_MASK;
	write_byte(nec_priv, nec_priv->imr1_bits, IMR1);
	write_byte(nec_priv, nec_priv->imr2_bits, IMR2);

	write_byte(nec_priv, AUX_PON, AUXMR);

	return 0;
}

void ni_isa_detach(gpib_device_t *device)
{
	nat4882_private_t *nat_priv = device->private_data;
	nec7210_private_t *nec_priv;

	if(nat_priv)
	{
		nec_priv = &nat_priv->nec7210_priv;
		if(nat_priv->irq)
		{
			free_irq(nat_priv->irq, device);
		}
		if(nec_priv->iobase)
		{
			nec7210_board_reset(nec_priv);
			release_region(nec_priv->iobase, atgpib_iosize);
		}
	}
	nat4882_free_private(device);
}

