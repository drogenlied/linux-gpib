/***************************************************************************
                          tnt4882-tms/init.c  -  description
                             -------------------
 board specific initialization stuff for National Instruments boards
 using tnt4882, this driver puts the board into tms9914 compatibility mode
 so I can test support for tms9914 compatible chips.


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

#include "tnt4882.h"
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <asm/dma.h>
#include <gpib_buffer.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/string.h>

#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

int ni_pci_attach(gpib_device_t *device);

void ni_pci_detach(gpib_device_t *device);

// wrappers for interface functions
ssize_t tnt4882_read(gpib_device_t *device, uint8_t *buffer, size_t length, int *end)
{
	tnt4882_private_t *priv = device->private_data;
	return tms9914_read(device, &priv->tms9914_priv, buffer, length, end);
}
ssize_t tnt4882_write(gpib_device_t *device, uint8_t *buffer, size_t length, int send_eoi)
{
	tnt4882_private_t *priv = device->private_data;
	return tms9914_write(device, &priv->tms9914_priv, buffer, length, send_eoi);
}
ssize_t tnt4882_command(gpib_device_t *device, uint8_t *buffer, size_t length)
{
	tnt4882_private_t *priv = device->private_data;
	return tms9914_command(device, &priv->tms9914_priv, buffer, length);
}
int tnt4882_take_control(gpib_device_t *device, int synchronous)
{
	tnt4882_private_t *priv = device->private_data;
	return tms9914_take_control(device, &priv->tms9914_priv, synchronous);
}
int tnt4882_go_to_standby(gpib_device_t *device)
{
	tnt4882_private_t *priv = device->private_data;
	return tms9914_go_to_standby(device, &priv->tms9914_priv);
}
void tnt4882_interface_clear(gpib_device_t *device, int assert)
{
	tnt4882_private_t *priv = device->private_data;
	tms9914_interface_clear(device, &priv->tms9914_priv, assert);
}
void tnt4882_remote_enable(gpib_device_t *device, int enable)
{
	tnt4882_private_t *priv = device->private_data;
	tms9914_remote_enable(device, &priv->tms9914_priv, enable);
}
void tnt4882_enable_eos(gpib_device_t *device, uint8_t eos_byte, int compare_8_bits)
{
	tnt4882_private_t *priv = device->private_data;
	tms9914_enable_eos(device, &priv->tms9914_priv, eos_byte, compare_8_bits);
}
void tnt4882_disable_eos(gpib_device_t *device)
{
	tnt4882_private_t *priv = device->private_data;
	tms9914_disable_eos(device, &priv->tms9914_priv);
}
unsigned int tnt4882_update_status(gpib_device_t *device)
{
	tnt4882_private_t *priv = device->private_data;
	return tms9914_update_status(device, &priv->tms9914_priv);
}
void tnt4882_primary_address(gpib_device_t *device, unsigned int address)
{
	tnt4882_private_t *priv = device->private_data;
	tms9914_primary_address(device, &priv->tms9914_priv, address);
}
void tnt4882_secondary_address(gpib_device_t *device, unsigned int address, int enable)
{
	tnt4882_private_t *priv = device->private_data;
	tms9914_secondary_address(device, &priv->tms9914_priv, address, enable);
}
int tnt4882_parallel_poll(gpib_device_t *device, uint8_t *result)
{
	tnt4882_private_t *priv = device->private_data;
	return tms9914_parallel_poll(device, &priv->tms9914_priv, result);
}
int tnt4882_serial_poll_response(gpib_device_t *device, uint8_t status)
{
	tnt4882_private_t *priv = device->private_data;
	return tms9914_serial_poll_response(device, &priv->tms9914_priv, status);
}

gpib_interface_t ni_pci_interface =
{
	name: "ni_pci",
	attach: ni_pci_attach,
	detach: ni_pci_detach,
	read: tnt4882_read,
	write: tnt4882_write,
	command: tnt4882_command,
	take_control: tnt4882_take_control,
	go_to_standby: tnt4882_go_to_standby,
	interface_clear: tnt4882_interface_clear,
	remote_enable: tnt4882_remote_enable,
	enable_eos: tnt4882_enable_eos,
	disable_eos: tnt4882_disable_eos,
	parallel_poll: tnt4882_parallel_poll,
	line_status: NULL,	//XXX
	update_status: tnt4882_update_status,
	primary_address: tnt4882_primary_address,
	secondary_address: tnt4882_secondary_address,
	serial_poll_response: tnt4882_serial_poll_response,
};

int tnt4882_allocate_private(gpib_device_t *device)
{
	device->private_data = kmalloc(sizeof(tnt4882_private_t), GFP_KERNEL);
	if(device->private_data == NULL)
		return -1;
	memset(device->private_data, 0, sizeof(tnt4882_private_t));
	return 0;
}

void tnt4882_free_private(gpib_device_t *device)
{
	if(device->private_data)
	{
		kfree(device->private_data);
		device->private_data = NULL;
	}
}

int ni_pci_attach(gpib_device_t *device)
{
	tnt4882_private_t *tnt_priv;
	tms9914_private_t *tms_priv;
	int isr_flags = 0;

	device->status = 0;

	if(tnt4882_allocate_private(device))
		return -ENOMEM;
	tnt_priv = device->private_data;
	tms_priv = &tnt_priv->tms9914_priv;
	tms_priv->read_byte = tms9914_iomem_read_byte;
	tms_priv->write_byte = tms9914_iomem_write_byte;
	tms_priv->offset = atgpib_reg_offset;

	if(mite_devices == NULL)
	{
		printk("no National Instruments PCI boards found\n");
		return -1;
	}

	for(tnt_priv->mite = mite_devices; tnt_priv->mite; tnt_priv->mite = tnt_priv->mite->next)
	{
		if(mite_device_id(tnt_priv->mite) == PCI_DEVICE_ID_NI_GPIB) break;
	}
	if(tnt_priv->mite == NULL)
	{
		printk("no NI PCI-GPIB boards found\n");
		return -1;
	}

	if(mite_setup(tnt_priv->mite) < 0)
	{
		printk("error setting up mite");
		return -1;
	}

	tms_priv->iobase = mite_iobase(tnt_priv->mite);

	// get irq
	if(request_irq(mite_irq(tnt_priv->mite), tnt4882_interrupt, isr_flags, "ni-pci-gpib", device))
	{
		printk("gpib: can't request IRQ %d\n", device->ibirq);
		return -1;
	}
	tnt_priv->irq = mite_irq(tnt_priv->mite);

	/* NAT 4882 reset */
	writeb(SFTRST, tms_priv->iobase + CMDR);	/* Turbo488 software reset */
	udelay(1);
	// enable system control
	writeb(SETSC, tms_priv->iobase + CMDR);
	udelay(1);

	// turn off one-chip mode
	writeb(NODMA, tms_priv->iobase + HSSEL);

	// make sure we are in 7210 mode
	write_byte(tms_priv, AUX_7210, AUXCR);
	// registers might be swapped, so write it to the swapped address too
	writeb(AUX_7210, tms_priv->iobase +  SWAPPED_AUXCR);
	udelay(1);

	// clear SWAP bit
	writeb(0x0, tms_priv->iobase + KEYREG);

printk("CSR 0x%x\n", readb(tms_priv->iobase + KEYREG));

	// put it in 9914 mode
	writeb(AUX_9914, tms_priv->iobase + AUXMR);

	// chip reset command
	write_byte(tms_priv, 0x1c, AUXCR);

	tms9914_board_reset(tms_priv);

	// enable passing of tms9914 interrupts
	writeb(0x2, tms_priv->iobase + IMR3);

	// enable interrupt
	writeb(0x1, tms_priv->iobase + INTRT);

	// enable tms9914 interrupts
	tms_priv->imr0_bits = HR_MACIE | HR_RLCIE | HR_ENDIE | HR_BOIE | HR_BIIE;
	tms_priv->imr1_bits = HR_MAIE | HR_SRQIE | HR_UNCIE | HR_ERRIE;
	write_byte(tms_priv, tms_priv->imr0_bits, IMR0);
	write_byte(tms_priv, tms_priv->imr1_bits, IMR1);
	write_byte(tms_priv, AUX_DAI, AUXCR);

	write_byte(tms_priv, AUX_CR, AUXCR);

printk("CSR 0x%x\n", readb(tms_priv->iobase + 0x17));

	return 0;
}

void ni_pci_detach(gpib_device_t *device)
{
	tnt4882_private_t *tnt_priv = device->private_data;
	tms9914_private_t *tms_priv;

	if(tnt_priv)
	{
		tms_priv = &tnt_priv->tms9914_priv;
		if(tnt_priv->irq)
		{
			free_irq(tnt_priv->irq, device);
		}
		if(tms_priv->iobase)
		{
			tms9914_board_reset(tms_priv);
		}
		if(tnt_priv->mite)
			mite_unsetup(tnt_priv->mite);
	}
	tnt4882_free_private(device);
}


int init_module(void)
{
	EXPORT_NO_SYMBOLS;

	INIT_LIST_HEAD(&ni_pci_interface.list);

	gpib_register_driver(&ni_pci_interface);

	mite_init();
	mite_list_devices();

	return 0;
}

void cleanup_module(void)
{
	gpib_unregister_driver(&ni_pci_interface);

	mite_cleanup();
}









