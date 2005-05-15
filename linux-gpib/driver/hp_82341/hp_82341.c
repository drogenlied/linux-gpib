/***************************************************************************
                          hp_82341/hp_82341.c  -  description
                             -------------------
Driver for hp 82341a/b/c/d boards.  Might be worth merging with Agilent
82350b driver.
    copyright            : (C) 2002, 2005 by Frank Mori Hess
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
#include <linux/isapnp.h>

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
	outb(priv->mode_control_bits, priv->tms9914_priv.iobase + MODE_CONTROL_STATUS_REG);
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
	return inb(priv->iobase + TMS9914_BASE_REG + register_num);
}

void hp_82341_write_byte( tms9914_private_t *priv, uint8_t data, unsigned int register_num )
{
	outb(data, priv->iobase + TMS9914_BASE_REG + register_num);
}

int hp_82341_find_isapnp_board(struct pnp_dev **dev)
{
	*dev = pnp_find_dev(NULL, ISAPNP_VENDOR('H', 'W', 'P'),
		ISAPNP_FUNCTION(0x1411), NULL );
	if(*dev == NULL || (*dev)->card == NULL)
	{
		printk( "hp_82341: failed to find isapnp board\n" );
		return -ENODEV;
	}
	if(pnp_device_attach(*dev) < 0)
 	{
		printk( "hp_82341: board already active, skipping\n" );
		return -EBUSY;
	}
	if(pnp_activate_dev(*dev) < 0 )
	{
		pnp_device_detach(*dev);
		printk( "hp_82341: failed to activate() atgpib/tnt, aborting\n" );
		return -EAGAIN;
	}
	if(!pnp_port_valid(*dev, 0) || !pnp_irq_valid(*dev, 0))
	{
		pnp_device_detach(*dev);
		printk( "hp_82341: invalid port or irq for atgpib/tnt, aborting\n" );
		return -ENOMEM;
	}
	return 0;
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

	if(board->ibbase == 0)
	{
		struct pnp_dev *dev;
		int retval = hp_82341_find_isapnp_board(&dev);
		if(retval < 0)
			return retval;
		hp_priv->pnp_dev = dev;
		board->ibbase = pnp_port_start(dev, 0);
		board->ibirq = pnp_irq(dev, 0);
	}
	if(request_region(board->ibbase, hp_82341_iosize, "hp_82341" ) == NULL)
	{
		printk( "hp_82341: failed to allocate io ports 0x%lx-0x%lx\n",
			board->ibbase,
			board->ibbase + hp_82341_iosize - 1);
		return -EIO;
	}
	tms_priv->iobase = board->ibbase;
	printk("hp_82341: base io 0x%lx\n", tms_priv->iobase);
	/* temporary hack to get board running until we figure out how to make
	 interrupts work */
	if(gpib_request_pseudo_irq(board, hp_82341_interrupt))
	{
		printk("hp_82341: failed to allocate pseudo_irq\n");
		return -EIO;
	}	
	if(request_irq(board->ibirq, hp_82341_interrupt, 0, "hp_82341", board))
	{
		printk( "hp_82341: failed to allocate IRQ %d\n", board->ibirq);
		return -EIO;
	}
	hp_priv->irq = board->ibirq;
	printk("hp_82341: IRQ %d\n", board->ibirq);
	printk("hp_82341: config status=0x%x\n", inb(tms_priv->iobase + CONFIG_CONTROL_STATUS_REG));
	hp_priv->mode_control_bits |= ENABLE_IRQ_CONFIG_BIT;
	outb(hp_priv->mode_control_bits, tms_priv->iobase + MODE_CONTROL_STATUS_REG);
	outb(IRQ_SELECT_BITS(board->ibirq), tms_priv->iobase + CONFIG_CONTROL_STATUS_REG);
	hp_priv->mode_control_bits &= ~ENABLE_IRQ_CONFIG_BIT;
	outb(hp_priv->mode_control_bits, tms_priv->iobase + MODE_CONTROL_STATUS_REG);
	tms9914_board_reset(tms_priv);
	outb(ENABLE_TI_INTERRUPT_EVENT_BIT, tms_priv->iobase +  EVENT_ENABLE_REG);
	outb(ENABLE_TI_INTERRUPT_BIT, tms_priv->iobase + INTERRUPT_ENABLE_REG);
	//write clear event register
	outb((TI_INTERRUPT_EVENT_BIT | POINTERS_EQUAL_EVENT_BIT |
		BUFFER_END_EVENT_BIT | TERMINAL_COUNT_EVENT_BIT),
		tms_priv->iobase + EVENT_STATUS_REG);	

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
		if(tms_priv->iobase)
		{
			outb(0, tms_priv->iobase + INTERRUPT_ENABLE_REG);
			tms9914_board_reset( tms_priv );
			if( hp_priv->irq )
			{
				free_irq(hp_priv->irq, board);
			}
			gpib_free_pseudo_irq(board);
			release_region(tms_priv->iobase, hp_82341_iosize);
		}
		if(hp_priv->pnp_dev)
		{			
			pnp_device_detach(hp_priv->pnp_dev);
		}
	}
	hp_82341_free_private( board );
}

static const struct pnp_device_id hp_82341_pnp_table[] __devinitdata = 
{
	{.id = "HWP1411"},
	{.id = ""}
};
MODULE_DEVICE_TABLE(pnp, hp_82341_pnp_table);

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
	hp_82341_private_t *hp_priv = board->private_data;
	tms9914_private_t *tms_priv = &hp_priv->tms9914_priv;
	unsigned long flags;
	irqreturn_t retval = IRQ_NONE;
	int event_status;
		
	spin_lock_irqsave( &board->spinlock, flags );
	event_status = inb(tms_priv->iobase + EVENT_STATUS_REG);
	printk("hp_82341: interrupt event_status=0x%x\n", event_status);
	if(event_status & INTERRUPT_PENDING_EVENT_BIT)
	{
		retval = IRQ_HANDLED;
	}
	//write-clear status bits
	if(event_status & (TI_INTERRUPT_EVENT_BIT | POINTERS_EQUAL_EVENT_BIT |
		BUFFER_END_EVENT_BIT | TERMINAL_COUNT_EVENT_BIT))
	{
		outb(event_status & (TI_INTERRUPT_EVENT_BIT | POINTERS_EQUAL_EVENT_BIT |
			BUFFER_END_EVENT_BIT | TERMINAL_COUNT_EVENT_BIT),
			tms_priv->iobase + EVENT_STATUS_REG);
	}
	if(event_status & TI_INTERRUPT_EVENT_BIT)
	{
		status1 = read_byte(tms_priv, ISR0);
		status2 = read_byte(tms_priv, ISR1);
		tms9914_interrupt_have_status(board, tms_priv, status1, status2);
		printk("hp_82341: interrupt status1=0x%x status2=0x%x\n",
			status1, status2);
	}
	spin_unlock_irqrestore( &board->spinlock, flags );
	return retval;
}

