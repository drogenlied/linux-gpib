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

MODULE_LICENSE("GPL");

int ines_pci_attach(gpib_board_t *board);
int ines_pci_accel_attach(gpib_board_t *board);
int ines_isa_attach(gpib_board_t *board);

void ines_pci_detach(gpib_board_t *board);
void ines_isa_detach(gpib_board_t *board);

enum ines_pci_chip
{
	PCI_CHIP_PLX9050,
	PCI_CHIP_AMCC5920,
};

typedef struct
{
	unsigned int vendor_id;
	unsigned int device_id;
	unsigned int subsystem_vendor_id;
	unsigned int subsystem_device_id;
	unsigned int gpib_region;
	enum ines_pci_chip pci_chip_type;
} ines_pci_id;

ines_pci_id pci_ids[] =
{
	{
		vendor_id: PCI_VENDOR_ID_PLX,
		device_id: PCI_DEVICE_ID_PLX_9050,
		subsystem_vendor_id: PCI_VENDOR_ID_PLX,
		subsystem_device_id: 0x1072,
		gpib_region: 2,
		pci_chip_type: PCI_CHIP_PLX9050,
	},
	{
		vendor_id: PCI_VENDOR_ID_AMCC,
		device_id: 0x8507,
		subsystem_vendor_id: PCI_VENDOR_ID_AMCC,
		subsystem_device_id: 0x1072,
		gpib_region: 1,
		pci_chip_type: PCI_CHIP_AMCC5920,
	},
};

static const int num_pci_chips = sizeof(pci_ids) / sizeof(pci_ids[0]);

// wrappers for interface functions
ssize_t ines_read(gpib_board_t *board, uint8_t *buffer, size_t length, int *end)
{
	ines_private_t *priv = board->private_data;
	return nec7210_read(board, &priv->nec7210_priv, buffer, length, end);
}
ssize_t ines_write(gpib_board_t *board, uint8_t *buffer, size_t length, int send_eoi)
{
	ines_private_t *priv = board->private_data;
	return nec7210_write(board, &priv->nec7210_priv, buffer, length, send_eoi);
}
ssize_t ines_command(gpib_board_t *board, uint8_t *buffer, size_t length)
{
	ines_private_t *priv = board->private_data;
	return nec7210_command(board, &priv->nec7210_priv, buffer, length);
}
int ines_take_control(gpib_board_t *board, int synchronous)
{
	ines_private_t *priv = board->private_data;
	return nec7210_take_control(board, &priv->nec7210_priv, synchronous);
}
int ines_go_to_standby(gpib_board_t *board)
{
	ines_private_t *priv = board->private_data;
	return nec7210_go_to_standby(board, &priv->nec7210_priv);
}
void ines_request_system_control( gpib_board_t *board, int request_control )
{
	ines_private_t *priv = board->private_data;
	nec7210_request_system_control( board, &priv->nec7210_priv, request_control );
}
void ines_interface_clear(gpib_board_t *board, int assert)
{
	ines_private_t *priv = board->private_data;
	nec7210_interface_clear(board, &priv->nec7210_priv, assert);
}
void ines_remote_enable(gpib_board_t *board, int enable)
{
	ines_private_t *priv = board->private_data;
	nec7210_remote_enable(board, &priv->nec7210_priv, enable);
}
void ines_enable_eos(gpib_board_t *board, uint8_t eos_byte, int compare_8_bits)
{
	ines_private_t *priv = board->private_data;
	nec7210_enable_eos(board, &priv->nec7210_priv, eos_byte, compare_8_bits);
}
void ines_disable_eos(gpib_board_t *board)
{
	ines_private_t *priv = board->private_data;
	nec7210_disable_eos(board, &priv->nec7210_priv);
}
unsigned int ines_update_status(gpib_board_t *board)
{
	ines_private_t *priv = board->private_data;
	return nec7210_update_status(board, &priv->nec7210_priv);
}
void ines_primary_address(gpib_board_t *board, unsigned int address)
{
	ines_private_t *priv = board->private_data;
	nec7210_primary_address(board, &priv->nec7210_priv, address);
}
void ines_secondary_address(gpib_board_t *board, unsigned int address, int enable)
{
	ines_private_t *priv = board->private_data;
	nec7210_secondary_address(board, &priv->nec7210_priv, address, enable);
}
int ines_parallel_poll(gpib_board_t *board, uint8_t *result)
{
	ines_private_t *priv = board->private_data;
	return nec7210_parallel_poll(board, &priv->nec7210_priv, result);
}
void ines_parallel_poll_response(gpib_board_t *board, uint8_t config)
{
	ines_private_t *priv = board->private_data;
	nec7210_parallel_poll_response(board, &priv->nec7210_priv, config);
}
void ines_serial_poll_response(gpib_board_t *board, uint8_t status)
{
	ines_private_t *priv = board->private_data;
	nec7210_serial_poll_response(board, &priv->nec7210_priv, status);
}
uint8_t ines_serial_poll_status( gpib_board_t *board )
{
	ines_private_t *priv = board->private_data;
	return nec7210_serial_poll_status( board, &priv->nec7210_priv );
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
	request_system_control: ines_request_system_control,
	interface_clear: ines_interface_clear,
	remote_enable: ines_remote_enable,
	enable_eos: ines_enable_eos,
	disable_eos: ines_disable_eos,
	parallel_poll: ines_parallel_poll,
	parallel_poll_response: ines_parallel_poll_response,
	line_status: ines_line_status,
	update_status: ines_update_status,
	primary_address: ines_primary_address,
	secondary_address: ines_secondary_address,
	serial_poll_response: ines_serial_poll_response,
	serial_poll_status: ines_serial_poll_status,
	provider_module: &__this_module,
};

gpib_interface_t ines_pci_accel_interface =
{
	name: "ines_pci_accel",
	attach: ines_pci_accel_attach,
	detach: ines_pci_detach,
	read: ines_accel_read,
	write: ines_accel_write,
	command: ines_command,
	take_control: ines_take_control,
	go_to_standby: ines_go_to_standby,
	request_system_control: ines_request_system_control,
	interface_clear: ines_interface_clear,
	remote_enable: ines_remote_enable,
	enable_eos: ines_enable_eos,
	disable_eos: ines_disable_eos,
	parallel_poll: ines_parallel_poll,
	parallel_poll_response: ines_parallel_poll_response,
	line_status: ines_line_status,
	update_status: ines_update_status,
	primary_address: ines_primary_address,
	secondary_address: ines_secondary_address,
	serial_poll_response: ines_serial_poll_response,
	serial_poll_status: ines_serial_poll_status,
	provider_module: &__this_module,
};

int ines_allocate_private(gpib_board_t *board)
{
	board->private_data = kmalloc(sizeof(ines_private_t), GFP_KERNEL);
	if(board->private_data == NULL)
		return -1;
	memset(board->private_data, 0, sizeof(ines_private_t));
	return 0;
}

void ines_free_private(gpib_board_t *board)
{
	if(board->private_data)
	{
		kfree(board->private_data);
		board->private_data = NULL;
	}
}

int ines_generic_attach(gpib_board_t *board)
{
	ines_private_t *ines_priv;
	nec7210_private_t *nec_priv;

	board->status = 0;

	if(ines_allocate_private(board))
		return -ENOMEM;
	ines_priv = board->private_data;
	nec_priv = &ines_priv->nec7210_priv;
	nec_priv->read_byte = nec7210_ioport_read_byte;
	nec_priv->write_byte = nec7210_ioport_write_byte;
	nec_priv->offset = ines_reg_offset;

	return 0;
}

void ines_online( ines_private_t *ines_priv, const gpib_board_t *board, int use_accel )
{
	nec7210_private_t *nec_priv = &ines_priv->nec7210_priv;

	/* set internal counter register for 8 MHz input clock */
	write_byte( nec_priv, ICR | 8, AUXMR );

	write_byte( nec_priv, INES_AUX_XMODE, AUXMR );
	write_byte( nec_priv, INES_AUX_CLR_IN_FIFO, AUXMR );
	write_byte( nec_priv, INES_AUX_CLR_OUT_FIFO, AUXMR );
	write_byte( nec_priv, INES_AUXD | 0, AUXMR );
	outb( 0, nec_priv->iobase + XDMA_CONTROL );
	ines_priv->extend_mode_bits = 0;
	outb( ines_priv->extend_mode_bits, nec_priv->iobase + EXTEND_MODE );
	if( use_accel )
	{
		outb( 0x80, nec_priv->iobase + OUT_FIFO_WATERMARK );
		outb( 0x80, nec_priv->iobase + IN_FIFO_WATERMARK );
		outb( IFC_ACTIVE_BIT | FIFO_ERROR_BIT | XFER_COUNT_BIT,
			nec_priv->iobase + IMR3 );
		outb( IN_FIFO_WATERMARK_BIT | OUT_FIFO_WATERMARK_BIT |
			OUT_FIFO_EMPTY_BIT, nec_priv->iobase + IMR4 );
		nec7210_set_reg_bits( nec_priv, ADMR, IN_FIFO_ENABLE_BIT | OUT_FIFO_ENABLE_BIT, 1 );
	}else
	{
		nec7210_set_reg_bits( nec_priv, ADMR, IN_FIFO_ENABLE_BIT | OUT_FIFO_ENABLE_BIT, 0 );
		outb( IFC_ACTIVE_BIT | FIFO_ERROR_BIT, nec_priv->iobase + IMR3 );
		outb( 0, nec_priv->iobase + IMR4 );
	}

	nec7210_board_online( nec_priv, board );
}

int ines_common_pci_attach( gpib_board_t *board )
{
	ines_private_t *ines_priv;
	nec7210_private_t *nec_priv;
	int isr_flags = 0;
	int retval;
	ines_pci_id found_id;
	unsigned int i;

	retval = ines_generic_attach(board);
	if(retval) return retval;

	ines_priv = board->private_data;
	nec_priv = &ines_priv->nec7210_priv;

	// find board
	ines_priv->pci_device = NULL;
	for(i = 0; i < num_pci_chips && ines_priv->pci_device == NULL; i++)
	{
		do
		{
			ines_priv->pci_device = pci_find_subsys(pci_ids[i].vendor_id, pci_ids[i].device_id,
				pci_ids[i].subsystem_vendor_id, pci_ids[i].subsystem_device_id, ines_priv->pci_device);
			if( ines_priv->pci_device == NULL )
				break;
			if( board->pci_bus >=0 && board->pci_bus != ines_priv->pci_device->bus->number )
				continue;
			if( board->pci_slot >= 0 && board->pci_slot !=
				PCI_SLOT( ines_priv->pci_device->devfn ) )
				continue;
			found_id = pci_ids[i];
			break;
		}while( 1 );
	}
	if(ines_priv->pci_device == NULL)
	{
		printk("gpib: could not find ines PCI board\n");
		return -1;
	}

	if(pci_enable_device(ines_priv->pci_device))
	{
		printk("error enabling pci device\n");
		return -1;
	}

	if(pci_request_regions(ines_priv->pci_device, "ines-gpib"))
		return -1;

	switch(found_id.pci_chip_type)
	{
		case PCI_CHIP_PLX9050:
			ines_priv->plx_iobase = pci_resource_start(ines_priv->pci_device, 1);
			break;
		case PCI_CHIP_AMCC5920:
			ines_priv->amcc_iobase = pci_resource_start(ines_priv->pci_device, 0);
			break;
		default:
			printk("gpib: unknown chip type? (bug)\n");
			pci_release_regions(ines_priv->pci_device);
			return -1;
			break;
	}
	nec_priv->iobase = pci_resource_start(ines_priv->pci_device, found_id.gpib_region);

	nec7210_board_reset( nec_priv, board );

	isr_flags |= SA_SHIRQ;
	if(request_irq(ines_priv->pci_device->irq, ines_interrupt, isr_flags, "pci-gpib", board))
	{
		printk("gpib: can't request IRQ %d\n",ines_priv->pci_device->irq);
		return -1;
	}
	ines_priv->irq = ines_priv->pci_device->irq;

	// enable interrupts on pci chip
	if(ines_priv->plx_iobase)
		outl(LINTR1_EN_BIT | LINTR1_POLARITY_BIT | PCI_INTR_EN_BIT,
			ines_priv->plx_iobase + PLX_INTCSR_REG);
	else if(ines_priv->amcc_iobase)
	{
		static const int region = 1;
		static const int num_wait_states = 7;
		uint32_t bits;

		bits = amcc_prefetch_bits(region, PREFETCH_DISABLED);
		bits |= amcc_PTADR_mode_bit(region);
		bits |= amcc_disable_write_fifo_bit(region);
		bits |= amcc_wait_state_bits(region, num_wait_states);
		outl(bits, ines_priv->amcc_iobase + AMCC_PASS_THRU_REG);
		outl(AMCC_ADDON_INTR_ENABLE_BIT, ines_priv->amcc_iobase + AMCC_INTCS_REG);
	}
	ines_online( ines_priv, board, 0 );

	return 0;
}

int ines_pci_attach( gpib_board_t *board )
{
	ines_private_t *ines_priv;
	int retval;

	retval = ines_common_pci_attach( board );
	if( retval < 0 ) return retval;

	ines_priv = board->private_data;
	ines_online( ines_priv, board, 0 );

	return 0;
}

int ines_pci_accel_attach( gpib_board_t *board )
{
	ines_private_t *ines_priv;
	int retval;

	retval = ines_common_pci_attach( board );
	if( retval < 0 ) return retval;

	ines_priv = board->private_data;
	ines_online( ines_priv, board, 1 );

	return 0;
}

void ines_pci_detach(gpib_board_t *board)
{
	ines_private_t *ines_priv = board->private_data;
	nec7210_private_t *nec_priv;

	if(ines_priv)
	{
		nec_priv = &ines_priv->nec7210_priv;
		if(ines_priv->irq)
		{
			// disable amcc interrupts
			outl(0, ines_priv->plx_iobase + PLX_INTCSR_REG );
			free_irq(ines_priv->irq, board);
		}
		if(nec_priv->iobase)
		{
			nec7210_board_reset( nec_priv, board );
			pci_release_regions(ines_priv->pci_device);
		}
	}
	ines_free_private(board);
}

int init_module(void)
{
	int err = 0;

	EXPORT_NO_SYMBOLS;

	gpib_register_driver(&ines_pci_interface);
	gpib_register_driver(&ines_pci_accel_interface);

#if defined(GPIB_CONFIG_PCMCIA)
	gpib_register_driver(&ines_pcmcia_interface);
	gpib_register_driver(&ines_pcmcia_accel_interface);
	err += ines_pcmcia_init_module();
#endif
	if(err)
		return -1;

	return 0;
}

void cleanup_module(void)
{
	gpib_unregister_driver(&ines_pci_interface);
	gpib_unregister_driver(&ines_pci_accel_interface);
#if defined(GPIB_CONFIG_PCMCIA)
	gpib_unregister_driver(&ines_pcmcia_interface);
	gpib_unregister_driver(&ines_pcmcia_accel_interface);
	ines_pcmcia_cleanup_module();
#endif
}









