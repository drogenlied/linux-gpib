/***************************************************************************
                          hp82335/hp82335_init.c  -  description
                             -------------------

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

#include "hp82335.h"
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <asm/dma.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");

int hp82335_attach( gpib_board_t *board );

void hp82335_detach( gpib_board_t *board );

// wrappers for interface functions
ssize_t hp82335_read( gpib_board_t *board, uint8_t *buffer, size_t length, int *end )
{
	hp82335_private_t *priv = board->private_data;
	return tms9914_read( board, &priv->tms9914_priv, buffer, length, end );
}
ssize_t hp82335_write( gpib_board_t *board, uint8_t *buffer, size_t length, int send_eoi )
{
	hp82335_private_t *priv = board->private_data;
	return tms9914_write( board, &priv->tms9914_priv, buffer, length, send_eoi );
}
ssize_t hp82335_command( gpib_board_t *board, uint8_t *buffer, size_t length )
{
	hp82335_private_t *priv = board->private_data;
	return tms9914_command( board, &priv->tms9914_priv, buffer, length );
}
int hp82335_take_control( gpib_board_t *board, int synchronous )
{
	hp82335_private_t *priv = board->private_data;
	return tms9914_take_control( board, &priv->tms9914_priv, synchronous );
}
int hp82335_go_to_standby( gpib_board_t *board )
{
	hp82335_private_t *priv = board->private_data;
	return tms9914_go_to_standby( board, &priv->tms9914_priv );
}
void hp82335_interface_clear( gpib_board_t *board, int assert )
{
	hp82335_private_t *priv = board->private_data;
	tms9914_interface_clear( board, &priv->tms9914_priv, assert );
}
void hp82335_remote_enable( gpib_board_t *board, int enable )
{
	hp82335_private_t *priv = board->private_data;
	tms9914_remote_enable( board, &priv->tms9914_priv, enable );
}
void hp82335_enable_eos( gpib_board_t *board, uint8_t eos_byte, int compare_8_bits )
{
	hp82335_private_t *priv = board->private_data;
	tms9914_enable_eos( board, &priv->tms9914_priv, eos_byte, compare_8_bits );
}
void hp82335_disable_eos( gpib_board_t *board )
{
	hp82335_private_t *priv = board->private_data;
	tms9914_disable_eos( board, &priv->tms9914_priv );
}
unsigned int hp82335_update_status( gpib_board_t *board )
{
	hp82335_private_t *priv = board->private_data;
	return tms9914_update_status( board, &priv->tms9914_priv );
}
void hp82335_primary_address( gpib_board_t *board, unsigned int address )
{
	hp82335_private_t *priv = board->private_data;
	tms9914_primary_address( board, &priv->tms9914_priv, address );
}
void hp82335_secondary_address( gpib_board_t *board, unsigned int address, int enable )
{
	hp82335_private_t *priv = board->private_data;
	tms9914_secondary_address( board, &priv->tms9914_priv, address, enable );
}
int hp82335_parallel_poll( gpib_board_t *board, uint8_t *result )
{
	hp82335_private_t *priv = board->private_data;
	return tms9914_parallel_poll( board, &priv->tms9914_priv, result );
}
void hp82335_parallel_poll_response( gpib_board_t *board, uint8_t config )
{
	hp82335_private_t *priv = board->private_data;
	tms9914_parallel_poll_response( board, &priv->tms9914_priv, config );
}
void hp82335_serial_poll_response( gpib_board_t *board, uint8_t status )
{
	hp82335_private_t *priv = board->private_data;
	tms9914_serial_poll_response( board, &priv->tms9914_priv, status );
}
uint8_t hp82335_serial_poll_status( gpib_board_t *board )
{
	hp82335_private_t *priv = board->private_data;
	return tms9914_serial_poll_status( board, &priv->tms9914_priv );
}
int hp82335_line_status( const gpib_board_t *board )
{
	hp82335_private_t *priv = board->private_data;
	return tms9914_line_status( board, &priv->tms9914_priv );
}

gpib_interface_t hp82335_interface =
{
	name: "hp82335",
	attach: hp82335_attach,
	detach: hp82335_detach,
	read: hp82335_read,
	write: hp82335_write,
	command: hp82335_command,
	take_control: hp82335_take_control,
	go_to_standby: hp82335_go_to_standby,
	interface_clear: hp82335_interface_clear,
	remote_enable: hp82335_remote_enable,
	enable_eos: hp82335_enable_eos,
	disable_eos: hp82335_disable_eos,
	parallel_poll: hp82335_parallel_poll,
	line_status: hp82335_line_status,
	update_status: hp82335_update_status,
	primary_address: hp82335_primary_address,
	secondary_address: hp82335_secondary_address,
	serial_poll_response: hp82335_serial_poll_response,
	provider_module: &__this_module,
};

int hp82335_allocate_private( gpib_board_t *board )
{
	board->private_data = kmalloc( sizeof( hp82335_private_t ), GFP_KERNEL );
	if( board->private_data == NULL )
		return -1;
	memset( board->private_data, 0, sizeof( hp82335_private_t ) );
	return 0;
}

void hp82335_free_private( gpib_board_t *board )
{
	if( board->private_data )
	{
		kfree( board->private_data );
		board->private_data = NULL;
	}
}

static inline unsigned int tms9914_to_hp82335_offset( unsigned int register_num )
{
	return 0x3ff8 + register_num;
}

uint8_t hp82335_read_byte( tms9914_private_t *priv, unsigned int register_num )
{
	return tms9914_iomem_read_byte( priv, tms9914_to_hp82335_offset( register_num ) );
}

void hp82335_write_byte( tms9914_private_t *priv, uint8_t data, unsigned int register_num )
{
	tms9914_iomem_write_byte( priv, data, tms9914_to_hp82335_offset( register_num ) );
}

void hp82335_clear_interrupt( hp82335_private_t *hp_priv )
{
	tms9914_private_t *tms_priv = &hp_priv->tms9914_priv;
	writeb( 0, tms_priv->iobase + HPREG_INTR_CLEAR );
}

int hp82335_attach( gpib_board_t *board )
{
	hp82335_private_t *hp_priv;
	tms9914_private_t *tms_priv;
	int isr_flags;

	board->status = 0;

	if( hp82335_allocate_private( board ) )
		return -ENOMEM;
	hp_priv = board->private_data;
	tms_priv = &hp_priv->tms9914_priv;
	tms_priv->read_byte = hp82335_read_byte;
	tms_priv->write_byte = hp82335_write_byte;
	tms_priv->offset = 1;

	switch( board->ibbase )
	{
		case 0xc4000:
		case 0xc8000:
		case 0xcc000:
		case 0xd0000:
		case 0xd4000:
		case 0xd8000:
		case 0xdc000:
		case 0xe0000:
		case 0xe4000:
		case 0xe8000:
		case 0xec000:
		case 0xf0000:
		case 0xf4000:
		case 0xf8000:
		case 0xfc000:
			break;
		default:
			printk( "hp82335: invalid base io address 0x%lx\n", board->ibbase );
			return -1;
			break;
	}
	if( request_mem_region( board->ibbase + hp82335_rom_size,
		hp82335_iomem_size - hp82335_rom_size, "hp82335" ) == NULL )
	{
		printk( "hp82335: failed to allocate io memory region 0x%lx-0x%lx\n",
			board->ibbase + hp82335_rom_size,
			board->ibbase + hp82335_iomem_size - hp82335_rom_size - 1 );
		return -1;
	}
	hp_priv->raw_iobase = board->ibbase;
	tms_priv->iobase = ( unsigned long ) ioremap( board->ibbase, hp82335_iomem_size );
	printk("hp82335: base address 0x%x remapped to 0x%lx\n", hp_priv->raw_iobase,
		tms_priv->iobase );

	// get irq
	isr_flags = 0;
	if( request_irq( board->ibirq, hp82335_interrupt, isr_flags, "hp82335", board ) )
	{
		printk( "hp82335: can't request IRQ %d\n", board->ibirq );
		return -1;
	}
	hp_priv->irq = board->ibirq;
	printk( "hp82335: IRQ %d\n", board->ibirq );

	tms9914_board_reset(tms_priv);

	writeb( INTR_ENABLE, tms_priv->iobase + HPREG_CCR );

	// enable tms9914 interrupts
	tms_priv->imr0_bits = HR_MACIE | HR_RLCIE | HR_ENDIE | HR_BOIE | HR_BIIE;
	tms_priv->imr1_bits = HR_MAIE | HR_SRQIE | HR_UNCIE | HR_ERRIE;
	write_byte(tms_priv, tms_priv->imr0_bits, IMR0);
	write_byte(tms_priv, tms_priv->imr1_bits, IMR1);
	write_byte(tms_priv, AUX_DAI, AUXCR);

	hp82335_clear_interrupt( hp_priv );

	write_byte( tms_priv, AUX_CR, AUXCR );

	return 0;
}

void hp82335_detach(gpib_board_t *board)
{
	hp82335_private_t *hp_priv = board->private_data;
	tms9914_private_t *tms_priv;

	if( hp_priv )
	{
		tms_priv = &hp_priv->tms9914_priv;
		if( hp_priv->irq )
		{
			free_irq( hp_priv->irq, board );
		}
		if( tms_priv->iobase )
		{
			writeb( 0, tms_priv->iobase + HPREG_CCR );
			tms9914_board_reset( tms_priv );
			iounmap( ( void * ) tms_priv->iobase );
		}
		if( hp_priv->raw_iobase )
			release_mem_region( hp_priv->raw_iobase + hp82335_rom_size,
				hp82335_iomem_size - hp82335_rom_size );
	}
	hp82335_free_private( board );
}


int init_module(void)
{
	EXPORT_NO_SYMBOLS;

	gpib_register_driver(&hp82335_interface);

	return 0;
}

void cleanup_module(void)
{
	gpib_unregister_driver(&hp82335_interface);
}



/*
 * GPIB interrupt service routines
 */

void hp82335_interrupt(int irq, void *arg, struct pt_regs *registerp)
{
	gpib_board_t *board = arg;
	hp82335_private_t *priv = board->private_data;

	tms9914_interrupt(board, &priv->tms9914_priv);

	hp82335_clear_interrupt( priv );
}

