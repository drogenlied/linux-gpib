/***************************************************************************
                          hp_82341/hp_82341.c  -  description
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

#include "hp_82341.h"
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");

int hp_82341_attach( gpib_board_t *board );

void hp_82341_detach( gpib_board_t *board );

// wrappers for interface functions
ssize_t hp_82341_read( gpib_board_t *board, uint8_t *buffer, size_t length, int *end, int *nbytes)
{
	hp_82341_private_t *priv = board->private_data;
	return tms9914_read( board, &priv->tms9914_priv, buffer, length, end, nbytes);
}
ssize_t hp_82341_write( gpib_board_t *board, uint8_t *buffer, size_t length, int send_eoi )
{
	hp_82341_private_t *priv = board->private_data;
	return tms9914_write( board, &priv->tms9914_priv, buffer, length, send_eoi );
}
ssize_t hp_82341_command( gpib_board_t *board, uint8_t *buffer, size_t length )
{
	hp_82341_private_t *priv = board->private_data;
	return tms9914_command( board, &priv->tms9914_priv, buffer, length );
}
int hp_82341_take_control( gpib_board_t *board, int synchronous )
{
	hp_82341_private_t *priv = board->private_data;
	return tms9914_take_control( board, &priv->tms9914_priv, synchronous );
}
int hp_82341_go_to_standby( gpib_board_t *board )
{
	hp_82341_private_t *priv = board->private_data;
	return tms9914_go_to_standby( board, &priv->tms9914_priv );
}
void hp_82341_request_system_control( gpib_board_t *board, int request_control )
{
	hp_82341_private_t *priv = board->private_data;
	if(request_control)
		priv->mode_control_bits |= SYSTEM_CONTROLLER_BIT;
	else
		priv->mode_control_bits &= ~SYSTEM_CONTROLLER_BIT;
	writeb(priv->mode_control_bits, priv->tms9914_priv.iobase + MODE_CONTROL_STATUS_REG);
	tms9914_request_system_control( board, &priv->tms9914_priv, request_control );
}
void hp_82341_interface_clear( gpib_board_t *board, int assert )
{
	hp_82341_private_t *priv = board->private_data;
	tms9914_interface_clear( board, &priv->tms9914_priv, assert );
}
void hp_82341_remote_enable( gpib_board_t *board, int enable )
{
	hp_82341_private_t *priv = board->private_data;
	tms9914_remote_enable( board, &priv->tms9914_priv, enable );
}
void hp_82341_enable_eos( gpib_board_t *board, uint8_t eos_byte, int compare_8_bits )
{
	hp_82341_private_t *priv = board->private_data;
	tms9914_enable_eos( board, &priv->tms9914_priv, eos_byte, compare_8_bits );
}
void hp_82341_disable_eos( gpib_board_t *board )
{
	hp_82341_private_t *priv = board->private_data;
	tms9914_disable_eos( board, &priv->tms9914_priv );
}
unsigned int hp_82341_update_status( gpib_board_t *board, unsigned int clear_mask )
{
	hp_82341_private_t *priv = board->private_data;
	return tms9914_update_status( board, &priv->tms9914_priv, clear_mask );
}
void hp_82341_primary_address( gpib_board_t *board, unsigned int address )
{
	hp_82341_private_t *priv = board->private_data;
	tms9914_primary_address( board, &priv->tms9914_priv, address );
}
void hp_82341_secondary_address( gpib_board_t *board, unsigned int address, int enable )
{
	hp_82341_private_t *priv = board->private_data;
	tms9914_secondary_address( board, &priv->tms9914_priv, address, enable );
}
int hp_82341_parallel_poll( gpib_board_t *board, uint8_t *result )
{
	hp_82341_private_t *priv = board->private_data;
	return tms9914_parallel_poll( board, &priv->tms9914_priv, result );
}
void hp_82341_parallel_poll_configure( gpib_board_t *board, uint8_t config )
{
	hp_82341_private_t *priv = board->private_data;
	tms9914_parallel_poll_configure( board, &priv->tms9914_priv, config );
}
void hp_82341_parallel_poll_response( gpib_board_t *board, int ist )
{
	hp_82341_private_t *priv = board->private_data;
	tms9914_parallel_poll_response( board, &priv->tms9914_priv, ist );
}
void hp_82341_serial_poll_response( gpib_board_t *board, uint8_t status )
{
	hp_82341_private_t *priv = board->private_data;
	tms9914_serial_poll_response( board, &priv->tms9914_priv, status );
}
uint8_t hp_82341_serial_poll_status( gpib_board_t *board )
{
	hp_82341_private_t *priv = board->private_data;
	return tms9914_serial_poll_status( board, &priv->tms9914_priv );
}
int hp_82341_line_status( const gpib_board_t *board )
{
	hp_82341_private_t *priv = board->private_data;
	return tms9914_line_status( board, &priv->tms9914_priv );
}
unsigned int hp_82341_t1_delay( gpib_board_t *board, unsigned int nano_sec )
{
	hp_82341_private_t *priv = board->private_data;
	return tms9914_t1_delay( board, &priv->tms9914_priv, nano_sec );
}
void hp_82341_return_to_local( gpib_board_t *board )
{
	hp_82341_private_t *priv = board->private_data;
	tms9914_return_to_local( board, &priv->tms9914_priv );
}

gpib_interface_t hp_82341_interface =
{
	name: "hp_82341",
	attach: hp_82341_attach,
	detach: hp_82341_detach,
	read: hp_82341_read,
	write: hp_82341_write,
	command: hp_82341_command,
	request_system_control: hp_82341_request_system_control,
	take_control: hp_82341_take_control,
	go_to_standby: hp_82341_go_to_standby,
	interface_clear: hp_82341_interface_clear,
	remote_enable: hp_82341_remote_enable,
	enable_eos: hp_82341_enable_eos,
	disable_eos: hp_82341_disable_eos,
	parallel_poll: hp_82341_parallel_poll,
	parallel_poll_configure: hp_82341_parallel_poll_configure,
	parallel_poll_response: hp_82341_parallel_poll_response,
	line_status: hp_82341_line_status,
	update_status: hp_82341_update_status,
	primary_address: hp_82341_primary_address,
	secondary_address: hp_82341_secondary_address,
	serial_poll_response: hp_82341_serial_poll_response,
	t1_delay: hp_82341_t1_delay,
	return_to_local: hp_82341_return_to_local,
};

int hp_82341_allocate_private( gpib_board_t *board )
{
	board->private_data = kmalloc( sizeof( hp_82341_private_t ), GFP_KERNEL );
	if( board->private_data == NULL )
		return -1;
	memset( board->private_data, 0, sizeof( hp_82341_private_t ) );
	return 0;
}

void hp_82341_free_private( gpib_board_t *board )
{
	if( board->private_data )
	{
		kfree( board->private_data );
		board->private_data = NULL;
	}
}

uint8_t hp_82341_read_byte( tms9914_private_t *priv, unsigned int register_num )
{
	return readb(priv->iobase + 0x10 + register_num);
}

void hp_82341_write_byte( tms9914_private_t *priv, uint8_t data, unsigned int register_num )
{
	writeb(data, priv->iobase + 0x800 + register_num);
}

void hp_82341_clear_interrupt( hp_82341_private_t *hp_priv )
{
	tms9914_private_t *tms_priv = &hp_priv->tms9914_priv;
	writeb(TI_INTERRUPT_EVENT_BIT, tms_priv->iobase + EVENT_STATUS_REG);
}

int hp_82341_attach( gpib_board_t *board )
{
	hp_82341_private_t *hp_priv;
	tms9914_private_t *tms_priv;

	board->status = 0;

	if( hp_82341_allocate_private( board ) )
		return -ENOMEM;
	hp_priv = board->private_data;
	tms_priv = &hp_priv->tms9914_priv;
	tms_priv->read_byte = hp_82341_read_byte;
	tms_priv->write_byte = hp_82341_write_byte;
	tms_priv->offset = 1;

	if(request_mem_region(board->ibbase, hp_82341_iomem_size, "hp_82341" ) == NULL)
	{
		printk( "hp_82341: failed to allocate io memory region 0x%lx-0x%lx\n",
			board->ibbase,
			board->ibbase + hp_82341_iomem_size - 1);
		return -EIO;
	}
	hp_priv->raw_iobase = board->ibbase;
	tms_priv->iobase = (unsigned long) ioremap(board->ibbase, hp_82341_iomem_size);
	printk("hp_82341: base address 0x%lx remapped to 0x%lx\n", hp_priv->raw_iobase,
		tms_priv->iobase );
	if(board->ibirq > 7 || board->ibirq < 1)
	{
		printk("hp_82341: bad irq=%i, must be in the range 1 through 7\n", board->ibirq);
		return -EINVAL;
	}
	if(request_irq(board->ibirq, hp_82341_interrupt, 0, "hp_82341", board))
	{
		printk( "hp_82341: failed to allocate IRQ %d\n", board->ibirq);
		return -EIO;
	}
	hp_priv->irq = board->ibirq;
	printk("hp_82341: IRQ %d\n", board->ibirq);
	hp_priv->mode_control_bits |= ENABLE_IRQ_CONFIG_BIT;
	writeb(hp_priv->mode_control_bits, tms_priv->iobase + MODE_CONTROL_STATUS_REG);
	writeb(IRQ_SELECT_BITS(board->ibirq), tms_priv->iobase + CONFIG_CONTROL_STATUS_REG);
	tms9914_board_reset(tms_priv);

	hp_82341_clear_interrupt( hp_priv );

	writeb(ENABLE_TI_INTERRUPT_BIT, tms_priv->iobase + INTERRUPT_ENABLE_REG);

	tms9914_online( board, tms_priv );

	return 0;
}

void hp_82341_detach(gpib_board_t *board)
{
	hp_82341_private_t *hp_priv = board->private_data;
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
			writeb(0, tms_priv->iobase + INTERRUPT_ENABLE_REG);
			tms9914_board_reset( tms_priv );
			iounmap( ( void * ) tms_priv->iobase );
		}
		if(hp_priv->raw_iobase)
			release_mem_region(hp_priv->raw_iobase,
				hp_82341_iomem_size);
	}
	hp_82341_free_private( board );
}


static int hp_82341_init_module( void )
{
	gpib_register_driver(&hp_82341_interface, &__this_module);
	return 0;
}

static void hp_82341_exit_module( void )
{
	gpib_unregister_driver(&hp_82341_interface);
}

module_init( hp_82341_init_module );
module_exit( hp_82341_exit_module );

/*
 * GPIB interrupt service routines
 */

irqreturn_t hp_82341_interrupt(int irq, void *arg, struct pt_regs *registerp)
{
	int status1, status2;
	gpib_board_t *board = arg;
	hp_82341_private_t *priv = board->private_data;
	unsigned long flags;
	irqreturn_t retval;
	
	spin_lock_irqsave( &board->spinlock, flags );
	status1 = read_byte( &priv->tms9914_priv, ISR0);
	status2 = read_byte( &priv->tms9914_priv, ISR1);
	hp_82341_clear_interrupt( priv );
	retval = tms9914_interrupt_have_status(board, &priv->tms9914_priv, status1, status2);
	spin_unlock_irqrestore( &board->spinlock, flags );
	return retval;
}

