/***************************************************************************
                          nec7210/tnt4882_init.c  -  description
                             -------------------
 board specific initialization stuff for National Instruments boards
 using tnt4882 or compatible chips (at-gpib, etc).

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
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/string.h>

#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

int ni_isa_attach(gpib_board_t *board);
int ni_pci_attach(gpib_board_t *board);

void ni_isa_detach(gpib_board_t *board);
void ni_pci_detach(gpib_board_t *board);

// wrappers for interface functions
ssize_t tnt4882_read(gpib_board_t *board, uint8_t *buffer, size_t length, int *end)
{
	tnt4882_private_t *priv = board->private_data;
	return nec7210_read(board, &priv->nec7210_priv, buffer, length, end);
}
ssize_t tnt4882_write(gpib_board_t *board, uint8_t *buffer, size_t length, int send_eoi)
{
	tnt4882_private_t *priv = board->private_data;
	return nec7210_write(board, &priv->nec7210_priv, buffer, length, send_eoi);
}
ssize_t tnt4882_command(gpib_board_t *board, uint8_t *buffer, size_t length)
{
	tnt4882_private_t *priv = board->private_data;
	return nec7210_command(board, &priv->nec7210_priv, buffer, length);
}
int tnt4882_take_control(gpib_board_t *board, int synchronous)
{
	tnt4882_private_t *priv = board->private_data;
	return nec7210_take_control(board, &priv->nec7210_priv, synchronous);
}
int tnt4882_go_to_standby(gpib_board_t *board)
{
	tnt4882_private_t *priv = board->private_data;
	return nec7210_go_to_standby(board, &priv->nec7210_priv);
}
void tnt4882_interface_clear(gpib_board_t *board, int assert)
{
	tnt4882_private_t *priv = board->private_data;
	nec7210_interface_clear(board, &priv->nec7210_priv, assert);
}
void tnt4882_remote_enable(gpib_board_t *board, int enable)
{
	tnt4882_private_t *priv = board->private_data;
	nec7210_remote_enable(board, &priv->nec7210_priv, enable);
}
void tnt4882_enable_eos(gpib_board_t *board, uint8_t eos_byte, int compare_8_bits)
{
	tnt4882_private_t *priv = board->private_data;
	nec7210_enable_eos(board, &priv->nec7210_priv, eos_byte, compare_8_bits);
}
void tnt4882_disable_eos(gpib_board_t *board)
{
	tnt4882_private_t *priv = board->private_data;
	nec7210_disable_eos(board, &priv->nec7210_priv);
}
unsigned int tnt4882_update_status(gpib_board_t *board)
{
	tnt4882_private_t *priv = board->private_data;
	return nec7210_update_status(board, &priv->nec7210_priv);
}
void tnt4882_primary_address(gpib_board_t *board, unsigned int address)
{
	tnt4882_private_t *priv = board->private_data;
	nec7210_primary_address(board, &priv->nec7210_priv, address);
}
void tnt4882_secondary_address(gpib_board_t *board, unsigned int address, int enable)
{
	tnt4882_private_t *priv = board->private_data;
	nec7210_secondary_address(board, &priv->nec7210_priv, address, enable);
}
int tnt4882_parallel_poll(gpib_board_t *board, uint8_t *result)
{
	tnt4882_private_t *priv = board->private_data;
	return nec7210_parallel_poll(board, &priv->nec7210_priv, result);
}
void tnt4882_parallel_poll_response(gpib_board_t *board, uint8_t config )
{
	tnt4882_private_t *priv = board->private_data;
	nec7210_parallel_poll_response( board, &priv->nec7210_priv, config );
}
// XXX tnt4882 has fancier serial poll capability, should send reqt AUX command
void tnt4882_serial_poll_response(gpib_board_t *board, uint8_t status)
{
	tnt4882_private_t *priv = board->private_data;
	nec7210_serial_poll_response(board, &priv->nec7210_priv, status);
}
uint8_t tnt4882_serial_poll_status( gpib_board_t *board )
{
	tnt4882_private_t *priv = board->private_data;
	return nec7210_serial_poll_status( board, &priv->nec7210_priv );
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
	parallel_poll_response: tnt4882_parallel_poll_response,
	line_status: NULL,	//XXX
	update_status: tnt4882_update_status,
	primary_address: tnt4882_primary_address,
	secondary_address: tnt4882_secondary_address,
	serial_poll_response: tnt4882_serial_poll_response,
	serial_poll_status: tnt4882_serial_poll_status,
	provider_module: &__this_module,
};

gpib_interface_t ni_isa_interface =
{
	name: "ni_isa",
	attach: ni_isa_attach,
	detach: ni_isa_detach,
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
	parallel_poll_response: tnt4882_parallel_poll_response,
	line_status: NULL,	//XXX
	update_status: tnt4882_update_status,
	primary_address: tnt4882_primary_address,
	secondary_address: tnt4882_secondary_address,
	serial_poll_response: tnt4882_serial_poll_response,
	serial_poll_status: tnt4882_serial_poll_status,
	provider_module: &__this_module,
};

int tnt4882_allocate_private(gpib_board_t *board)
{
	board->private_data = kmalloc(sizeof(tnt4882_private_t), GFP_KERNEL);
	if(board->private_data == NULL)
		return -1;
	memset(board->private_data, 0, sizeof(tnt4882_private_t));
	return 0;
}

void tnt4882_free_private(gpib_board_t *board)
{
	if(board->private_data)
	{
		kfree(board->private_data);
		board->private_data = NULL;
	}
}

void tnt4882_init( tnt4882_private_t *tnt_priv, const gpib_board_t *board )
{
	nec7210_private_t *nec_priv = &tnt_priv->nec7210_priv;
	const unsigned long iobase = nec_priv->iobase;

	/* NAT 4882 reset */
	udelay(1);
	tnt_priv->io_write( SFTRST, iobase + CMDR );	/* Turbo488 software reset */
	udelay(1);
	tnt_priv->io_write(SETSC, iobase + CMDR);

	// turn off one-chip mode
	tnt_priv->io_write(NODMA, iobase + HSSEL);

	// make sure we are in 7210 mode
	tnt_priv->io_write(AUX_7210, iobase + AUXCR);
	udelay(1);
	// registers might be swapped, so write it to the swapped address too
	tnt_priv->io_write(AUX_7210, iobase +  SWAPPED_AUXCR);
	udelay(1);

	nec7210_board_reset( &tnt_priv->nec7210_priv, board );

	// turn off one chip mode and dma
	tnt_priv->io_write( NODMA , iobase + HSSEL);

	// enable passing of nec7210 interrupts
	tnt_priv->io_write(0x2, iobase + IMR3);

	// enable interrupt
	tnt_priv->io_write(0x1, iobase + INTRT);

	// enable nec7210 interrupts
	nec_priv->imr1_bits = HR_ERRIE | HR_DECIE | HR_ENDIE |
		HR_DETIE | HR_APTIE | HR_CPTIE | HR_DOIE | HR_DIIE;
	nec_priv->imr2_bits = IMR2_ENABLE_INTR_MASK;
	write_byte(nec_priv, nec_priv->imr1_bits, IMR1);
	write_byte(nec_priv, nec_priv->imr2_bits, IMR2);

	write_byte(nec_priv, AUX_PON, AUXMR);
}

int ni_pci_attach(gpib_board_t *board)
{
	tnt4882_private_t *tnt_priv;
	nec7210_private_t *nec_priv;
	int isr_flags = SA_SHIRQ;

	board->status = 0;

	if(tnt4882_allocate_private(board))
		return -ENOMEM;
	tnt_priv = board->private_data;
	tnt_priv->io_write = writeb_wrapper;
	tnt_priv->io_read = readb_wrapper;
	nec_priv = &tnt_priv->nec7210_priv;
	nec_priv->read_byte = nec7210_iomem_read_byte;
	nec_priv->write_byte = nec7210_iomem_write_byte;
	nec_priv->offset = atgpib_reg_offset;

	if(mite_devices == NULL)
	{
		printk("no National Instruments PCI boards found\n");
		return -1;
	}

	for(tnt_priv->mite = mite_devices; tnt_priv->mite; tnt_priv->mite = tnt_priv->mite->next)
	{
		if( mite_device_id( tnt_priv->mite ) == PCI_DEVICE_ID_NI_GPIB ||
			mite_device_id( tnt_priv->mite ) == PCI_DEVICE_ID_NI_GPIB_PLUS )
			break;
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

	nec_priv->iobase = mite_iobase(tnt_priv->mite);

	// get irq
	if(request_irq(mite_irq(tnt_priv->mite), tnt4882_interrupt, isr_flags, "ni-pci-gpib", board))
	{
		printk("gpib: can't request IRQ %d\n", mite_irq( tnt_priv->mite ) );
		return -1;
	}
	tnt_priv->irq = mite_irq(tnt_priv->mite);

	tnt4882_init( tnt_priv, board );

	return 0;
}

void ni_pci_detach(gpib_board_t *board)
{
	tnt4882_private_t *tnt_priv = board->private_data;
	nec7210_private_t *nec_priv;

	if(tnt_priv)
	{
		nec_priv = &tnt_priv->nec7210_priv;
		if(tnt_priv->irq)
		{
			free_irq(tnt_priv->irq, board);
		}
		if(nec_priv->iobase)
		{
			nec7210_board_reset( nec_priv, board );
		}
		if(tnt_priv->mite)
			mite_unsetup(tnt_priv->mite);
	}
	tnt4882_free_private(board);
	GPIB_DPRINTK( "ni_pci board offline\n" );
}

int ni_isa_attach(gpib_board_t *board)
{
	tnt4882_private_t *tnt_priv;
	nec7210_private_t *nec_priv;
	int isr_flags = 0;

	board->status = 0;

	if(tnt4882_allocate_private(board))
		return -ENOMEM;
	tnt_priv = board->private_data;
	tnt_priv->io_write = outb_wrapper;
	tnt_priv->io_read = inb_wrapper;
	nec_priv = &tnt_priv->nec7210_priv;
	nec_priv->read_byte = nec7210_ioport_read_byte;
	nec_priv->write_byte = nec7210_ioport_write_byte;
	nec_priv->offset = atgpib_reg_offset;

	// allocate ioports
	if( request_region(board->ibbase, atgpib_iosize, "atgpib") == 0)
	{
		printk("gpib: ioports are already in use");
		return -1;
	}
	nec_priv->iobase = board->ibbase;

	// get irq
	if(request_irq(board->ibirq, tnt4882_interrupt, isr_flags, "atgpib", board))
	{
		printk("gpib: can't request IRQ %d\n", board->ibirq);
		return -1;
	}
	tnt_priv->irq = board->ibirq;

	tnt4882_init( tnt_priv, board );

	return 0;
}

void ni_isa_detach(gpib_board_t *board)
{
	tnt4882_private_t *tnt_priv = board->private_data;
	nec7210_private_t *nec_priv;

	if(tnt_priv)
	{
		nec_priv = &tnt_priv->nec7210_priv;
		if(tnt_priv->irq)
		{
			free_irq(tnt_priv->irq, board);
		}
		if(nec_priv->iobase)
		{
			nec7210_board_reset( nec_priv, board );
			release_region(nec_priv->iobase, atgpib_iosize);
		}
	}
	tnt4882_free_private(board);
}

int init_module(void)
{
	EXPORT_NO_SYMBOLS;

	gpib_register_driver(&ni_isa_interface);
	gpib_register_driver(&ni_pci_interface);

#if defined(GPIB_CONFIG_PCMCIA)
	gpib_register_driver(&ni_pcmcia_interface);
	if( init_ni_gpib_cs() < 0 )
		return -1;
#endif

	mite_init();
	mite_list_devices();

	return 0;
}

void cleanup_module(void)
{
	gpib_unregister_driver(&ni_isa_interface);
	gpib_unregister_driver(&ni_pci_interface);
#if defined(GPIB_CONFIG_PCMCIA)
	gpib_unregister_driver(&ni_pcmcia_interface);
	exit_ni_gpib_cs();
#endif

	mite_cleanup();
}









