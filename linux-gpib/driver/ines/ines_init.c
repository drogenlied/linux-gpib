/***************************************************************************
                          nec7210/ines_init.c  -  description
                             -------------------
  Initialization stuff for ines GPIB boards

    copyright            : (C) 1999 Axel Dziemba (axel.dziemba@ines.de)
                           (C) 2002 by Frank Mori Hess
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

#include "ines.h"

#include <linux/pci.h>
#include <asm/io.h>
#include <linux/module.h>

#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

#define INES_VENDOR_ID 0x10b5
#define INES_DEV_ID    0x9050
#define INES_SUBID 0x107210b5L

int ines_pci_attach(gpib_device_t *device);
int ines_isa_attach(gpib_device_t *device);

void ines_pci_detach(gpib_device_t *device);
void ines_isa_detach(gpib_device_t *device);

// wrappers for interface functions
ssize_t ines_read(gpib_device_t *device, uint8_t *buffer, size_t length, int *end)
{
	ines_private_t *priv = device->private_data;
	return nec7210_read(device, &priv->nec7210_priv, buffer, length, end);
}
ssize_t ines_write(gpib_device_t *device, uint8_t *buffer, size_t length, int send_eoi)
{
	ines_private_t *priv = device->private_data;
	return nec7210_write(device, &priv->nec7210_priv, buffer, length, send_eoi);
}
ssize_t ines_command(gpib_device_t *device, uint8_t *buffer, size_t length)
{
	ines_private_t *priv = device->private_data;
	return nec7210_command(device, &priv->nec7210_priv, buffer, length);
}
int ines_take_control(gpib_device_t *device, int synchronous)
{
	ines_private_t *priv = device->private_data;
	return nec7210_take_control(device, &priv->nec7210_priv, synchronous);
}
int ines_go_to_standby(gpib_device_t *device)
{
	ines_private_t *priv = device->private_data;
	return nec7210_go_to_standby(device, &priv->nec7210_priv);
}
void ines_interface_clear(gpib_device_t *device, int assert)
{
	ines_private_t *priv = device->private_data;
	nec7210_interface_clear(device, &priv->nec7210_priv, assert);
}
void ines_remote_enable(gpib_device_t *device, int enable)
{
	ines_private_t *priv = device->private_data;
	nec7210_remote_enable(device, &priv->nec7210_priv, enable);
}
void ines_enable_eos(gpib_device_t *device, uint8_t eos_byte, int compare_8_bits)
{
	ines_private_t *priv = device->private_data;
	nec7210_enable_eos(device, &priv->nec7210_priv, eos_byte, compare_8_bits);
}
void ines_disable_eos(gpib_device_t *device)
{
	ines_private_t *priv = device->private_data;
	nec7210_disable_eos(device, &priv->nec7210_priv);
}
unsigned int ines_update_status(gpib_device_t *device)
{
	ines_private_t *priv = device->private_data;
	return nec7210_update_status(device, &priv->nec7210_priv);
}
void ines_primary_address(gpib_device_t *device, unsigned int address)
{
	ines_private_t *priv = device->private_data;
	nec7210_primary_address(device, &priv->nec7210_priv, address);
}
void ines_secondary_address(gpib_device_t *device, unsigned int address, int enable)
{
	ines_private_t *priv = device->private_data;
	nec7210_secondary_address(device, &priv->nec7210_priv, address, enable);
}
int ines_parallel_poll(gpib_device_t *device, uint8_t *result)
{
	ines_private_t *priv = device->private_data;
	return nec7210_parallel_poll(device, &priv->nec7210_priv, result);
}
int ines_serial_poll_response(gpib_device_t *device, uint8_t status)
{
	ines_private_t *priv = device->private_data;
	return nec7210_serial_poll_response(device, &priv->nec7210_priv, status);
}

gpib_interface_t ines_pci_interface =
{
	name: "ines_pci",
	attach: ines_pci_attach,
	detach: ines_pci_detach,
	read: ines_read,
	write: ines_write,
	command: ines_command,
	take_control: ines_take_control,
	go_to_standby: ines_go_to_standby,
	interface_clear: ines_interface_clear,
	remote_enable: ines_remote_enable,
	enable_eos: ines_enable_eos,
	disable_eos: ines_disable_eos,
	parallel_poll: ines_parallel_poll,
	line_status: NULL,	//XXX
	update_status: ines_update_status,
	primary_address: ines_primary_address,
	secondary_address: ines_secondary_address,
	serial_poll_response: ines_serial_poll_response,
};

int ines_allocate_private(gpib_device_t *device)
{
	device->private_data = kmalloc(sizeof(ines_private_t), GFP_KERNEL);
	if(device->private_data == NULL)
		return -1;
	memset(device->private_data, 0, sizeof(ines_private_t));
	return 0;
}

void ines_free_private(gpib_device_t *device)
{
	if(device->private_data)
	{
		kfree(device->private_data);
		device->private_data = NULL;
	}
}

int ines_generic_attach(gpib_device_t *device)
{
	ines_private_t *ines_priv;
	nec7210_private_t *nec_priv;

	device->status = 0;

	if(ines_allocate_private(device))
		return -ENOMEM;
	ines_priv = device->private_data;
	nec_priv = &ines_priv->nec7210_priv;
	nec_priv->read_byte = nec7210_ioport_read_byte;
	nec_priv->write_byte = nec7210_ioport_write_byte;
	nec_priv->offset = ines_reg_offset;

	return 0;
}

void ines_init(ines_private_t *ines_priv)
{
	nec7210_private_t *nec_priv = &ines_priv->nec7210_priv;

	nec7210_board_reset(nec_priv);

	// enable interrupts for 7210
	nec_priv->imr1_bits = HR_ERRIE | HR_DECIE | HR_ENDIE |
		HR_DETIE | HR_APTIE | HR_CPTIE | HR_DOIE | HR_DIIE;
	nec_priv->imr2_bits = IMR2_ENABLE_INTR_MASK;
	write_byte(nec_priv, nec_priv->imr1_bits, IMR1);
	write_byte(nec_priv, nec_priv->imr2_bits, IMR2);

	write_byte(nec_priv, AUX_PON, AUXMR);
}

int ines_pci_attach(gpib_device_t *device)
{
	ines_private_t *ines_priv;
	nec7210_private_t *nec_priv;
	int isr_flags = 0;
	int retval;

	retval = ines_generic_attach(device);
	if(retval) return retval;

	ines_priv = device->private_data;
	nec_priv = &ines_priv->nec7210_priv;

	// find board
	ines_priv->pci_device = NULL;
	while((ines_priv->pci_device = pci_find_device(INES_VENDOR_ID, INES_DEV_ID, ines_priv->pci_device)))
	{
		// check for board with PLX PCI controller but not ines GPIB PCI board
		if(ines_priv->pci_device->subsystem_device == INES_SUBID)
		{
			break;
		}
	}
	if(ines_priv->pci_device == NULL)
	{
		printk("gpib: no ines PCI board found\n");
		return -1;
	}

	if(pci_enable_device(ines_priv->pci_device))
	{
		printk("error enabling pci device\n");
		return -1;
	}

	if(pci_request_regions(ines_priv->pci_device, "ines-gpib"))
		return -1;

	ines_priv->plx_iobase = pci_resource_start(ines_priv->pci_device, 1) & PCI_BASE_ADDRESS_IO_MASK;
	nec_priv->iobase = pci_resource_start(ines_priv->pci_device, 2) & PCI_BASE_ADDRESS_IO_MASK;

	isr_flags |= SA_SHIRQ;
	if(request_irq(ines_priv->pci_device->irq, ines_interrupt, isr_flags, "pci-gpib", device))
	{
		printk("gpib: can't request IRQ %d\n",ines_priv->pci_device->irq);
		return -1;
	}
	ines_priv->irq = ines_priv->pci_device->irq;

	// enable interrupts on plx chip
	outl(LINTR1_EN_BIT | LINTR1_POLARITY_BIT | PCI_INTR_EN_BIT,
		ines_priv->plx_iobase + PLX_INTCSR_REG);

	ines_init(ines_priv);

	return 0;
}

void ines_pci_detach(gpib_device_t *device)
{
	ines_private_t *ines_priv = device->private_data;
	nec7210_private_t *nec_priv;

	if(ines_priv)
	{
		nec_priv = &ines_priv->nec7210_priv;
		if(ines_priv->irq)
		{
			// disable amcc interrupts
			outl(0, ines_priv->plx_iobase + PLX_INTCSR_REG );
			free_irq(ines_priv->irq, device);
		}
		if(nec_priv->iobase)
		{
			nec7210_board_reset(nec_priv);
			pci_release_regions(ines_priv->pci_device);
		}
	}
	ines_free_private(device);
}

int init_module(void)
{
	int err = 0;

	EXPORT_NO_SYMBOLS;

	INIT_LIST_HEAD(&ines_pci_interface.list);

	gpib_register_driver(&ines_pci_interface);

#ifdef CONFIG_PCMCIA
	INIT_LIST_HEAD(&ines_pcmcia_interface.list);

	gpib_register_driver(&ines_pcmcia_interface);
	err += ines_pcmcia_init_module();
#endif
	if(err)
		return -1;

	return 0;
}

void cleanup_module(void)
{
	gpib_unregister_driver(&ines_pci_interface);
#ifdef CONFIG_PCMCIA
	gpib_unregister_driver(&ines_pcmcia_interface);
	ines_pcmcia_cleanup_module();
#endif
}









