/***************************************************************************
                          nec7210/cb7210_init.c  -  description
                             -------------------
 board specific initialization stuff for Measurement Computing boards
 using cb7210.2 and cbi488.2 chips

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

#include "cb7210.h"
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <asm/dma.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/string.h>

#define PCI_DEVICE_ID_CBOARDS_PCI_GPIB 0x6

#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

int cb_pci_attach(gpib_board_t *board);
int cb_isa_attach(gpib_board_t *board);

void cb_pci_detach(gpib_board_t *board);
void cb_isa_detach(gpib_board_t *board);

// wrappers for interface functions
ssize_t cb7210_read(gpib_board_t *board, uint8_t *buffer, size_t length, int *end)
{
	cb7210_private_t *priv = board->private_data;
	return nec7210_read(board, &priv->nec7210_priv, buffer, length, end);
}
ssize_t cb7210_write(gpib_board_t *board, uint8_t *buffer, size_t length, int send_eoi)
{
	cb7210_private_t *priv = board->private_data;
	return nec7210_write(board, &priv->nec7210_priv, buffer, length, send_eoi);
}
ssize_t cb7210_command(gpib_board_t *board, uint8_t *buffer, size_t length)
{
	cb7210_private_t *priv = board->private_data;
	return nec7210_command(board, &priv->nec7210_priv, buffer, length);
}
int cb7210_take_control(gpib_board_t *board, int synchronous)
{
	cb7210_private_t *priv = board->private_data;
	return nec7210_take_control(board, &priv->nec7210_priv, synchronous);
}
int cb7210_go_to_standby(gpib_board_t *board)
{
	cb7210_private_t *priv = board->private_data;
	return nec7210_go_to_standby(board, &priv->nec7210_priv);
}
void cb7210_interface_clear(gpib_board_t *board, int assert)
{
	cb7210_private_t *priv = board->private_data;
	nec7210_interface_clear(board, &priv->nec7210_priv, assert);
}
void cb7210_remote_enable(gpib_board_t *board, int enable)
{
	cb7210_private_t *priv = board->private_data;
	nec7210_remote_enable(board, &priv->nec7210_priv, enable);
}
void cb7210_enable_eos(gpib_board_t *board, uint8_t eos_byte, int compare_8_bits)
{
	cb7210_private_t *priv = board->private_data;
	nec7210_enable_eos(board, &priv->nec7210_priv, eos_byte, compare_8_bits);
}
void cb7210_disable_eos(gpib_board_t *board)
{
	cb7210_private_t *priv = board->private_data;
	nec7210_disable_eos(board, &priv->nec7210_priv);
}
unsigned int cb7210_update_status(gpib_board_t *board)
{
	cb7210_private_t *priv = board->private_data;
	return nec7210_update_status(board, &priv->nec7210_priv);
}
void cb7210_primary_address(gpib_board_t *board, unsigned int address)
{
	cb7210_private_t *priv = board->private_data;
	nec7210_primary_address(board, &priv->nec7210_priv, address);
}
void cb7210_secondary_address(gpib_board_t *board, unsigned int address, int enable)
{
	cb7210_private_t *priv = board->private_data;
	nec7210_secondary_address(board, &priv->nec7210_priv, address, enable);
}
int cb7210_parallel_poll(gpib_board_t *board, uint8_t *result)
{
	cb7210_private_t *priv = board->private_data;
	return nec7210_parallel_poll(board, &priv->nec7210_priv, result);
}
int cb7210_serial_poll_response(gpib_board_t *board, uint8_t status)
{
	cb7210_private_t *priv = board->private_data;
	return nec7210_serial_poll_response(board, &priv->nec7210_priv, status);
}

gpib_interface_t cb_pci_interface =
{
	name: "cbi_pci",
	attach: cb_pci_attach,
	detach: cb_pci_detach,
	read: cb7210_read,
	write: cb7210_write,
	command: cb7210_command,
	take_control: cb7210_take_control,
	go_to_standby: cb7210_go_to_standby,
	interface_clear: cb7210_interface_clear,
	remote_enable: cb7210_remote_enable,
	enable_eos: cb7210_enable_eos,
	disable_eos: cb7210_disable_eos,
	parallel_poll: cb7210_parallel_poll,
	line_status: NULL,	//XXX
	update_status: cb7210_update_status,
	primary_address: cb7210_primary_address,
	secondary_address: cb7210_secondary_address,
	serial_poll_response: cb7210_serial_poll_response,
};

gpib_interface_t cb_isa_interface =
{
	name: "cbi_isa",
	attach: cb_isa_attach,
	detach: cb_isa_detach,
	read: cb7210_read,
	write: cb7210_write,
	command: cb7210_command,
	take_control: cb7210_take_control,
	go_to_standby: cb7210_go_to_standby,
	interface_clear: cb7210_interface_clear,
	remote_enable: cb7210_remote_enable,
	enable_eos: cb7210_enable_eos,
	disable_eos: cb7210_disable_eos,
	parallel_poll: cb7210_parallel_poll,
	line_status: NULL,	//XXX
	update_status: cb7210_update_status,
	primary_address: cb7210_primary_address,
	secondary_address: cb7210_secondary_address,
	serial_poll_response: cb7210_serial_poll_response,
};

int cb7210_allocate_private(gpib_board_t *board)
{
	board->private_data = kmalloc(sizeof(cb7210_private_t), GFP_KERNEL);
	if(board->private_data == NULL)
		return -1;
	memset(board->private_data, 0, sizeof(cb7210_private_t));
	return 0;
}

void cb7210_free_private(gpib_board_t *board)
{
	if(board->private_data)
	{
		kfree(board->private_data);
		board->private_data = NULL;
	}
}

// generic part of attach functions shared by all cb7210 boards
int cb_generic_attach(gpib_board_t *board)
{
	cb7210_private_t *cb_priv;
	nec7210_private_t *nec_priv;

	board->status = 0;

	if(cb7210_allocate_private(board))
		return -ENOMEM;
	cb_priv = board->private_data;
	nec_priv = &cb_priv->nec7210_priv;
	nec_priv->read_byte = nec7210_ioport_read_byte;
	nec_priv->write_byte = nec7210_ioport_write_byte;
	nec_priv->offset = cb7210_reg_offset;

	return 0;
}

void cb7210_init(cb7210_private_t *cb_priv)
{
	nec7210_private_t *nec_priv = &cb_priv->nec7210_priv;

	nec7210_board_reset(nec_priv);

	// XXX set clock register for 20MHz? driving frequency
	write_byte(nec_priv, ICR | 8, AUXMR);

	// enable nec7210 interrupts
	nec_priv->imr1_bits = HR_ERRIE | HR_DECIE | HR_ENDIE |
		HR_DETIE | HR_APTIE | HR_CPTIE | HR_DOIE | HR_DIIE;
	nec_priv->imr2_bits = IMR2_ENABLE_INTR_MASK;
	write_byte(nec_priv, nec_priv->imr1_bits, IMR1);
	write_byte(nec_priv, nec_priv->imr2_bits, IMR2);

	write_byte(nec_priv, AUX_PON, AUXMR);
}

int cb_pci_attach(gpib_board_t *board)
{
	cb7210_private_t *cb_priv;
	nec7210_private_t *nec_priv;
	int isr_flags = 0;
	int bits;
	int retval;

	retval = cb_generic_attach(board);
	if(retval) return retval;

	cb_priv = board->private_data;
	nec_priv = &cb_priv->nec7210_priv;
	
	// find board
	cb_priv->pci_device = pci_find_device(PCI_VENDOR_ID_CBOARDS, PCI_DEVICE_ID_CBOARDS_PCI_GPIB, NULL);
	if(cb_priv->pci_device == NULL)
	{
		printk("GPIB: no PCI-GPIB board found\n");
		return -1;
	}

	if(pci_enable_device(cb_priv->pci_device))
	{
		printk("error enabling pci device\n");
		return -1;
	}

	if(pci_request_regions(cb_priv->pci_device, "pci-gpib"))
		return -1;

	cb_priv->amcc_iobase = pci_resource_start(cb_priv->pci_device, 0) & PCI_BASE_ADDRESS_IO_MASK;
	nec_priv->iobase = pci_resource_start(cb_priv->pci_device, 1) & PCI_BASE_ADDRESS_IO_MASK;

	/* CBI 4882 reset */
	write_byte(nec_priv, HS_RESET7210, HS_INT_LEVEL);
	write_byte(nec_priv, 0, HS_INT_LEVEL);
	write_byte(nec_priv, 0, HS_MODE); /* disable system control */

	isr_flags |= SA_SHIRQ;
	if(request_irq(cb_priv->pci_device->irq, cb_pci_interrupt, isr_flags, "pci-gpib", board))
	{
		printk("gpib: can't request IRQ %d\n",cb_priv->pci_device->irq);
		return -1;
	}
	cb_priv->irq = cb_priv->pci_device->irq;

	// make sure mailbox flags are clear
	inl(cb_priv->amcc_iobase + INCOMING_MAILBOX_REG(3));
	// enable interrupts on amccs5933 chip
	bits = INBOX_FULL_INTR_BIT | INBOX_BYTE_BITS(3) | INBOX_SELECT_BITS(3) |
		INBOX_INTR_CS_BIT;
	outl(bits, cb_priv->amcc_iobase + INTCSR_REG );

	cb7210_init(cb_priv);

	return 0;
}

void cb_pci_detach(gpib_board_t *board)
{
	cb7210_private_t *cb_priv = board->private_data;
	nec7210_private_t *nec_priv;

	if(cb_priv)
	{
		nec_priv = &cb_priv->nec7210_priv;
		if(cb_priv->irq)
		{
			// disable amcc interrupts
			outl(0, cb_priv->amcc_iobase + INTCSR_REG );
			free_irq(cb_priv->irq, board);
		}
		if(nec_priv->iobase)
		{
			nec7210_board_reset(nec_priv);
			pci_release_regions(cb_priv->pci_device);
		}
	}
	cb7210_free_private(board);
}

/* Returns bits to be sent to base+9 to configure irq level on isa-gpib.
 * Returns zero on failure.
 */
unsigned int intr_level_bits(unsigned int irq)
{
	switch(irq)
	{
		case 2:
		case 3:
		case 4:
		case 5:
			return irq - 1;
			break;
		case 7:
			return 0x5;
			break;
		case 10:
			return 0x6;
			break;
		case 11:
			return 0x7;
			break;
		default:
			return 0;
			break;
	}
}

int cb_isa_attach(gpib_board_t *board)
{
	int isr_flags = 0;
	cb7210_private_t *cb_priv;
	nec7210_private_t *nec_priv;
	unsigned int irq_bits;
	int retval;

	retval = cb_generic_attach(board);
	if(retval) return retval;

	if(request_region(board->ibbase, cb7210_iosize, "isa-gpib"));
	{
		printk("gpib: ioports are already in use");
		return -1;
	}
	nec_priv->iobase = board->ibbase;

	irq_bits = intr_level_bits(board->ibirq);
	if(irq_bits == 0)
	{
		printk("board incapable of using irq %i, try 2-5, 7, 10, or 11\n", board->ibirq);
	}

	// put in nec7210 compatibility mode and configure board irq
	write_byte(nec_priv, HS_RESET7210, HS_INT_LEVEL);
	write_byte(nec_priv, irq_bits, HS_INT_LEVEL);
	write_byte(nec_priv, 0, HS_MODE); /* disable system control */

	// install interrupt handler
	if(request_irq(board->ibirq, cb7210_interrupt, isr_flags, "isa-gpib", board))
	{
		printk("gpib: can't request IRQ %d\n", board->ibirq);
		return -1;
	}
	cb_priv->irq = board->ibirq;

	cb7210_init(cb_priv);

	return 0;
}

void cb_isa_detach(gpib_board_t *board)
{
	cb7210_private_t *cb_priv = board->private_data;
	nec7210_private_t *nec_priv;

	if(cb_priv)
	{
		nec_priv = &cb_priv->nec7210_priv;
		if(cb_priv->irq)
		{
			free_irq(cb_priv->irq, board);
		}
		if(nec_priv->iobase)
		{
			nec7210_board_reset(nec_priv);
			release_region(nec_priv->iobase, cb7210_iosize);
		}
	}
	cb7210_free_private(board);
}

int init_module(void)
{
	int err = 0;

	EXPORT_NO_SYMBOLS;

	INIT_LIST_HEAD(&cb_pci_interface.list);
	INIT_LIST_HEAD(&cb_isa_interface.list);

	gpib_register_driver(&cb_pci_interface);
	gpib_register_driver(&cb_isa_interface);

#if defined(CONFIG_PCMCIA) || defined(CONFIG_PCMCIA_MODULE)
	INIT_LIST_HEAD(&cb_pcmcia_interface.list);

	gpib_register_driver(&cb_pcmcia_interface);
	err += cb_pcmcia_init_module();
#endif
	if(err)
		return -1;

	return 0;
}

void cleanup_module(void)
{
	gpib_unregister_driver(&cb_pci_interface);
	gpib_unregister_driver(&cb_isa_interface);
#if defined(CONFIG_PCMCIA) || defined(CONFIG_PCMCIA_MODULE)
	gpib_unregister_driver(&cb_pcmcia_interface);
	cb_pcmcia_cleanup_module();
#endif
}









