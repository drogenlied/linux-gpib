/***************************************************************************
                          nec7210/pc2_init.c  -  description
                             -------------------
 initialization for pc2 and pc2a compatible boards

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

int pc2_attach(gpib_device_t *device);
int pc2a_attach(gpib_device_t *device);

void pc2_detach(gpib_device_t *device);
void pc2a_detach(gpib_device_t *device);

// wrappers for interface functions
ssize_t pc2_read(gpib_device_t *device, uint8_t *buffer, size_t length, int *end)
{
	pc2_private_t *priv = device->private_data;
	return nec7210_read(device, &priv->nec7210_priv, buffer, length, end);
}
ssize_t pc2_write(gpib_device_t *device, uint8_t *buffer, size_t length, int send_eoi)
{
	pc2_private_t *priv = device->private_data;
	return nec7210_write(device, &priv->nec7210_priv, buffer, length, send_eoi);
}
ssize_t pc2_command(gpib_device_t *device, uint8_t *buffer, size_t length)
{
	pc2_private_t *priv = device->private_data;
	return nec7210_command(device, &priv->nec7210_priv, buffer, length);
}
int pc2_take_control(gpib_device_t *device, int synchronous)
{
	pc2_private_t *priv = device->private_data;
	return nec7210_take_control(device, &priv->nec7210_priv, synchronous);
}
int pc2_go_to_standby(gpib_device_t *device)
{
	pc2_private_t *priv = device->private_data;
	return nec7210_go_to_standby(device, &priv->nec7210_priv);
}
void pc2_interface_clear(gpib_device_t *device, int assert)
{
	pc2_private_t *priv = device->private_data;
	nec7210_interface_clear(device, &priv->nec7210_priv, assert);
}
void pc2_remote_enable(gpib_device_t *device, int enable)
{
	pc2_private_t *priv = device->private_data;
	nec7210_remote_enable(device, &priv->nec7210_priv, enable);
}
void pc2_enable_eos(gpib_device_t *device, uint8_t eos_byte, int compare_8_bits)
{
	pc2_private_t *priv = device->private_data;
	nec7210_enable_eos(device, &priv->nec7210_priv, eos_byte, compare_8_bits);
}
void pc2_disable_eos(gpib_device_t *device)
{
	pc2_private_t *priv = device->private_data;
	nec7210_disable_eos(device, &priv->nec7210_priv);
}
unsigned int pc2_update_status(gpib_device_t *device)
{
	pc2_private_t *priv = device->private_data;
	return nec7210_update_status(device, &priv->nec7210_priv);
}
void pc2_primary_address(gpib_device_t *device, unsigned int address)
{
	pc2_private_t *priv = device->private_data;
	nec7210_primary_address(device, &priv->nec7210_priv, address);
}
void pc2_secondary_address(gpib_device_t *device, unsigned int address, int enable)
{
	pc2_private_t *priv = device->private_data;
	nec7210_secondary_address(device, &priv->nec7210_priv, address, enable);
}
int pc2_parallel_poll(gpib_device_t *device, uint8_t *result)
{
	pc2_private_t *priv = device->private_data;
	return nec7210_parallel_poll(device, &priv->nec7210_priv, result);
}
int pc2_serial_poll_response(gpib_device_t *device, uint8_t status)
{
	pc2_private_t *priv = device->private_data;
	return nec7210_serial_poll_response(device, &priv->nec7210_priv, status);
}

gpib_interface_t pc2_interface =
{
	name:	"nec7210",
	attach:	pc2_attach,
	detach:	pc2_detach,
	read:	pc2_read,
	write:	pc2_write,
	command:	pc2_command,
	take_control:	pc2_take_control,
	go_to_standby:	pc2_go_to_standby,
	interface_clear:	pc2_interface_clear,
	remote_enable:	pc2_remote_enable,
	enable_eos:	pc2_enable_eos,
	disable_eos:	pc2_disable_eos,
	parallel_poll:	pc2_parallel_poll,
	line_status:	NULL,
	update_status:	pc2_update_status,
	primary_address:	pc2_primary_address,
	secondary_address:	pc2_secondary_address,
	serial_poll_response:	pc2_serial_poll_response,
};

gpib_interface_t pc2a_interface =
{
	name:	"nec7210",
	attach:	pc2a_attach,
	detach:	pc2a_detach,
	read:	pc2_read,
	write:	pc2_write,
	command:	pc2_command,
	take_control:	pc2_take_control,
	go_to_standby:	pc2_go_to_standby,
	interface_clear:	pc2_interface_clear,
	remote_enable:	pc2_remote_enable,
	enable_eos:	pc2_enable_eos,
	disable_eos:	pc2_disable_eos,
	parallel_poll:	pc2_parallel_poll,
	line_status:	NULL,
	update_status:	pc2_update_status,
	primary_address:	pc2_primary_address,
	secondary_address:	pc2_secondary_address,
	serial_poll_response:	pc2_serial_poll_response,
};

static int allocate_private(gpib_device_t *device)
{
	device->private_data = kmalloc(sizeof(pc2_private_t), GFP_KERNEL);
	if(device->private_data == NULL)
		return -1;
	memset(device->private_data, 0, sizeof(pc2_private_t));
	return 0;
}

static void free_private(gpib_device_t *device)
{
	if(device->private_data)
	{
		kfree(device->private_data);
		device->private_data = NULL;
	}
}

int pc2_attach(gpib_device_t *device)
{
	int isr_flags = 0;
	pc2_private_t *pc2_priv;
	nec7210_private_t *nec_priv;
	device->status = 0;

	if(allocate_private(device))
		return -ENOMEM;
	pc2_priv = device->private_data;
	nec_priv = &pc2_priv->nec7210_priv;
	nec_priv->offset = pc2_reg_offset;
	nec_priv->read_byte = ioport_read_byte;
	nec_priv->write_byte = ioport_write_byte;

	if(request_region(ibbase, pc2_iosize, "pc2"));
	{
		printk("gpib: ioports are already in use");
		return -1;
	}
	nec_priv->iobase = ibbase;

	// install interrupt handler
	if( request_irq(ibirq, pc2_interrupt, isr_flags, "pc2", device))
	{
		printk("gpib: can't request IRQ %d\n", ibirq);
		return -1;
	}
	pc2_priv->irq = ibirq;

	// request isa dma channel
#if DMAOP
	if( request_dma( ibdma, "pc2" ) )
	{
		printk("gpib: can't request DMA %d\n",ibdma );
		return -1;
	}
	nec_priv->dma_channel = ibdma;
#endif
	nec7210_board_reset(nec_priv);

	// enable interrupts
	nec_priv->imr1_bits = HR_ERRIE | HR_DECIE |
		HR_DETIE | HR_APTIE | HR_CPTIE;
	nec_priv->imr2_bits = IMR2_ENABLE_INTR_MASK;
	nec_priv->write_byte(nec_priv, nec_priv->imr1_bits, IMR1);
	nec_priv->write_byte(nec_priv, nec_priv->imr2_bits, IMR2);

	nec_priv->write_byte(nec_priv, AUX_PON, AUXMR);

	return 0;
}

void pc2_detach(gpib_device_t *device)
{
	pc2_private_t *pc2_priv = device->private_data;
	nec7210_private_t *nec_priv;

	if(pc2_priv)
	{
		nec_priv = &pc2_priv->nec7210_priv;
		if(nec_priv->dma_channel)
		{
			free_dma(nec_priv->dma_channel);
		}
		if(pc2_priv->irq)
		{
			free_irq(pc2_priv->irq, device);
		}
		if(nec_priv->iobase)
		{
			nec7210_board_reset(nec_priv);
			release_region(nec_priv->iobase, pc2_iosize);
		}
	}
	free_private(device);
}

int pc2a_attach(gpib_device_t *device)
{
	unsigned int i, err;
	int isr_flags = 0;
	pc2_private_t *pc2_priv;
	nec7210_private_t *nec_priv;

	device->status = 0;

	if(allocate_private(device))
		return -ENOMEM;
	pc2_priv = device->private_data;
	nec_priv = &pc2_priv->nec7210_priv;
	nec_priv->offset = pc2a_reg_offset;
	nec_priv->read_byte = ioport_read_byte;
	nec_priv->write_byte = ioport_write_byte;

	switch( ibbase ){

		case 0x02e1:
		case 0x22e1:
		case 0x42e1:
		case 0x62e1:
			break;
		default:
			printk("PCIIa base range invalid, must be one of [0246]2e1 is %lx \n", ibbase);
			return -1;
			break;
	}

	if( ibirq < 2 || ibirq > 7 )
	{
		printk("Illegal Interrupt Level \n");
		return -1;
	}

	err = 0;
	for(i = 0; i < nec7210_num_registers; i++)
	{
		if(check_region(ibbase + i * pc2a_reg_offset, 1))
			err++;
	}
	if(check_region(pc2a_clear_intr_iobase, pc2a_clear_intr_iosize))
	{
		err++;
	}
	if(err)
	{
		printk("gpib: ioports are already in use");
		return -1;
	}
	for(i = 0; i < nec7210_num_registers; i++)
	{
		request_region(ibbase + i * pc2a_reg_offset, 1, "pc2a");
	}
	request_region(pc2a_clear_intr_iobase, pc2a_clear_intr_iosize, "pc2a");
	nec_priv->iobase = ibbase;

	if(request_irq(ibirq, pc2a_interrupt, isr_flags, "pc2a", device))
	{
		printk("gpib: can't request IRQ %d\n", ibirq);
		return -1;
	}
	pc2_priv->irq = ibirq;
	// request isa dma channel
#if DMAOP
	if(request_dma(ibdma, "pc2a"))
	{
		printk("gpib: can't request DMA %d\n",ibdma );
		return -1;
	}
	nec_priv->dma_channel = ibdma;
#endif
	nec7210_board_reset(nec_priv);

	// make sure interrupt is clear
	outb(0xff , CLEAR_INTR_REG(pc2_priv->irq));

	// enable interrupts
	nec_priv->imr1_bits = HR_ERRIE | HR_DECIE | HR_ENDIE |
		HR_DETIE | HR_APTIE | HR_CPTIE;
	nec_priv->imr2_bits = IMR2_ENABLE_INTR_MASK;
	nec_priv->write_byte(nec_priv, nec_priv->imr1_bits, IMR1);
	nec_priv->write_byte(nec_priv, nec_priv->imr2_bits, IMR2);

	nec_priv->write_byte(nec_priv, AUX_PON, AUXMR);

	return 0;
}

void pc2a_detach(gpib_device_t *device)
{
	int i;
	pc2_private_t *pc2_priv = device->private_data;
	nec7210_private_t *nec_priv;

	if(pc2_priv)
	{
		nec_priv = &pc2_priv->nec7210_priv;
		if(nec_priv->dma_channel)
		{
			free_dma(nec_priv->dma_channel);
		}
		if(pc2_priv->irq)
		{
			free_irq(pc2_priv->irq, device);
		}
		if(nec_priv->iobase)
		{
			nec7210_board_reset(nec_priv);
			for(i = 0; i < nec7210_num_registers; i++)
				release_region(nec_priv->iobase + i * pc2a_reg_offset, 1);
			release_region(pc2a_clear_intr_iobase, pc2a_clear_intr_iosize);
		}
	}
	free_private(device);
}

