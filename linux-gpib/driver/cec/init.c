/***************************************************************************
                          cec/init.c  -  description
                             -------------------
  Initialization stuff for Capital Equipment Corp. GPIB boards

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

#include "cec.h"

#include <linux/pci.h>
#include <asm/io.h>
#include <linux/module.h>

#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

#define CEC_VENDOR_ID 0x12fc
#define CEC_DEV_ID    0x5cec
#define CEC_SUBID 0x9050

int cec_pci_attach(gpib_device_t *device);

void cec_pci_detach(gpib_device_t *device);

// wrappers for interface functions
ssize_t cec_read(gpib_device_t *device, uint8_t *buffer, size_t length, int *end)
{
	cec_private_t *priv = device->private_data;
	return nec7210_read(device, &priv->nec7210_priv, buffer, length, end);
}
ssize_t cec_write(gpib_device_t *device, uint8_t *buffer, size_t length, int send_eoi)
{
	cec_private_t *priv = device->private_data;
	return nec7210_write(device, &priv->nec7210_priv, buffer, length, send_eoi);
}
ssize_t cec_command(gpib_device_t *device, uint8_t *buffer, size_t length)
{
	cec_private_t *priv = device->private_data;
	return nec7210_command(device, &priv->nec7210_priv, buffer, length);
}
int cec_take_control(gpib_device_t *device, int synchronous)
{
	cec_private_t *priv = device->private_data;
	return nec7210_take_control(device, &priv->nec7210_priv, synchronous);
}
int cec_go_to_standby(gpib_device_t *device)
{
	cec_private_t *priv = device->private_data;
	return nec7210_go_to_standby(device, &priv->nec7210_priv);
}
void cec_interface_clear(gpib_device_t *device, int assert)
{
	cec_private_t *priv = device->private_data;
	nec7210_interface_clear(device, &priv->nec7210_priv, assert);
}
void cec_remote_enable(gpib_device_t *device, int enable)
{
	cec_private_t *priv = device->private_data;
	nec7210_remote_enable(device, &priv->nec7210_priv, enable);
}
void cec_enable_eos(gpib_device_t *device, uint8_t eos_byte, int compare_8_bits)
{
	cec_private_t *priv = device->private_data;
	nec7210_enable_eos(device, &priv->nec7210_priv, eos_byte, compare_8_bits);
}
void cec_disable_eos(gpib_device_t *device)
{
	cec_private_t *priv = device->private_data;
	nec7210_disable_eos(device, &priv->nec7210_priv);
}
unsigned int cec_update_status(gpib_device_t *device)
{
	cec_private_t *priv = device->private_data;
	return nec7210_update_status(device, &priv->nec7210_priv);
}
void cec_primary_address(gpib_device_t *device, unsigned int address)
{
	cec_private_t *priv = device->private_data;
	nec7210_primary_address(device, &priv->nec7210_priv, address);
}
void cec_secondary_address(gpib_device_t *device, unsigned int address, int enable)
{
	cec_private_t *priv = device->private_data;
	nec7210_secondary_address(device, &priv->nec7210_priv, address, enable);
}
int cec_parallel_poll(gpib_device_t *device, uint8_t *result)
{
	cec_private_t *priv = device->private_data;
	return nec7210_parallel_poll(device, &priv->nec7210_priv, result);
}
int cec_serial_poll_response(gpib_device_t *device, uint8_t status)
{
	cec_private_t *priv = device->private_data;
	return nec7210_serial_poll_response(device, &priv->nec7210_priv, status);
}

gpib_interface_t cec_pci_interface =
{
	name: "cec_pci",
	attach: cec_pci_attach,
	detach: cec_pci_detach,
	read: cec_read,
	write: cec_write,
	command: cec_command,
	take_control: cec_take_control,
	go_to_standby: cec_go_to_standby,
	interface_clear: cec_interface_clear,
	remote_enable: cec_remote_enable,
	enable_eos: cec_enable_eos,
	disable_eos: cec_disable_eos,
	parallel_poll: cec_parallel_poll,
	line_status: NULL,	//XXX
	update_status: cec_update_status,
	primary_address: cec_primary_address,
	secondary_address: cec_secondary_address,
	serial_poll_response: cec_serial_poll_response,
};

int cec_allocate_private(gpib_device_t *device)
{
	device->private_data = kmalloc(sizeof(cec_private_t), GFP_KERNEL);
	if(device->private_data == NULL)
		return -1;
	memset(device->private_data, 0, sizeof(cec_private_t));
	return 0;
}

void cec_free_private(gpib_device_t *device)
{
	if(device->private_data)
	{
		kfree(device->private_data);
		device->private_data = NULL;
	}
}

int cec_generic_attach(gpib_device_t *device)
{
	cec_private_t *cec_priv;
	nec7210_private_t *nec_priv;

	device->status = 0;

	if(cec_allocate_private(device))
		return -ENOMEM;
	cec_priv = device->private_data;
	nec_priv = &cec_priv->nec7210_priv;
	nec_priv->read_byte = nec7210_ioport_read_byte;
	nec_priv->write_byte = nec7210_ioport_write_byte;
	nec_priv->offset = cec_reg_offset;

	return 0;
}

void cec_init(cec_private_t *cec_priv)
{
	nec7210_private_t *nec_priv = &cec_priv->nec7210_priv;

	nec7210_board_reset(nec_priv);

	// make interrupt active low
	nec_priv->auxb_bits |= HR_INV;
	write_byte(nec_priv, nec_priv->auxb_bits, AUXMR);

	// enable interrupts for 7210
	nec_priv->imr1_bits = HR_ERRIE | HR_DECIE | HR_ENDIE |
		HR_DETIE | HR_APTIE | HR_CPTIE | HR_DOIE | HR_DIIE;
	nec_priv->imr2_bits = IMR2_ENABLE_INTR_MASK;
	write_byte(nec_priv, nec_priv->imr1_bits, IMR1);
	write_byte(nec_priv, nec_priv->imr2_bits, IMR2);

	write_byte(nec_priv, AUX_PON, AUXMR);
}

int cec_pci_attach(gpib_device_t *device)
{
	cec_private_t *cec_priv;
	nec7210_private_t *nec_priv;
	int isr_flags = 0;
	int retval;

	retval = cec_generic_attach(device);
	if(retval) return retval;

	cec_priv = device->private_data;
	nec_priv = &cec_priv->nec7210_priv;

	// find board
	cec_priv->pci_device = NULL;
	while((cec_priv->pci_device = pci_find_device(CEC_VENDOR_ID, CEC_DEV_ID, cec_priv->pci_device)))
	{
		// check for board with plx9050 controller
		if(cec_priv->pci_device->subsystem_device == CEC_SUBID)
		{
			break;
		}
	}
	if(cec_priv->pci_device == NULL)
	{
		printk("gpib: no cec PCI board found\n");
		return -1;
	}

	if(pci_enable_device(cec_priv->pci_device))
	{
		printk("error enabling pci device\n");
		return -1;
	}

	if(pci_request_regions(cec_priv->pci_device, "cec-gpib"))
		return -1;

	cec_priv->plx_iobase = pci_resource_start(cec_priv->pci_device, 1) & PCI_BASE_ADDRESS_IO_MASK;
	nec_priv->iobase = pci_resource_start(cec_priv->pci_device, 2) & PCI_BASE_ADDRESS_IO_MASK;

	isr_flags |= SA_SHIRQ;
	if(request_irq(cec_priv->pci_device->irq, cec_interrupt, isr_flags, "pci-gpib", device))
	{
		printk("gpib: can't request IRQ %d\n",cec_priv->pci_device->irq);
		return -1;
	}
	cec_priv->irq = cec_priv->pci_device->irq;

	// enable interrupts on plx chip
	outl(LINTR1_EN_BIT | PCI_INTR_EN_BIT,
		cec_priv->plx_iobase + PLX_INTCSR_REG);

	cec_init(cec_priv);

	return 0;
}

void cec_pci_detach(gpib_device_t *device)
{
	cec_private_t *cec_priv = device->private_data;
	nec7210_private_t *nec_priv;

	if(cec_priv)
	{
		nec_priv = &cec_priv->nec7210_priv;
		if(cec_priv->irq)
		{
			// disable plx9050 interrupts
			outl(0, cec_priv->plx_iobase + PLX_INTCSR_REG );
			free_irq(cec_priv->irq, device);
		}
		if(nec_priv->iobase)
		{
			nec7210_board_reset(nec_priv);
			pci_release_regions(cec_priv->pci_device);
		}
	}
	cec_free_private(device);
}

int init_module(void)
{
	EXPORT_NO_SYMBOLS;

	INIT_LIST_HEAD(&cec_pci_interface.list);

	gpib_register_driver(&cec_pci_interface);

	return 0;
}

void cleanup_module(void)
{
	gpib_unregister_driver(&cec_pci_interface);
}









