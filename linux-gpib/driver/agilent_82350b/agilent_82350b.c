/***************************************************************************
                          agilent_82350b/agilent_82350b.c  -  description
                             -------------------

    copyright            : (C) 2002, 2004 by Frank Mori Hess
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

#include "agilent_82350b.h"
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <asm/dma.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/string.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");

int agilent_82350b_attach( gpib_board_t *board );

void agilent_82350b_detach( gpib_board_t *board );

// wrappers for interface functions
ssize_t agilent_82350b_read( gpib_board_t *board, uint8_t *buffer, size_t length, int *end, int *nbytes)
{
	agilent_82350b_private_t *priv = board->private_data;
	return tms9914_read( board, &priv->tms9914_priv, buffer, length, end, nbytes);
}
ssize_t agilent_82350b_write( gpib_board_t *board, uint8_t *buffer, size_t length, int send_eoi )
{
	agilent_82350b_private_t *priv = board->private_data;
	return tms9914_write( board, &priv->tms9914_priv, buffer, length, send_eoi );
}
ssize_t agilent_82350b_command( gpib_board_t *board, uint8_t *buffer, size_t length )
{
	agilent_82350b_private_t *priv = board->private_data;
	return tms9914_command( board, &priv->tms9914_priv, buffer, length );
}
int agilent_82350b_take_control( gpib_board_t *board, int synchronous )
{
	agilent_82350b_private_t *priv = board->private_data;
	return tms9914_take_control( board, &priv->tms9914_priv, synchronous );
}
int agilent_82350b_go_to_standby( gpib_board_t *board )
{
	agilent_82350b_private_t *priv = board->private_data;
	return tms9914_go_to_standby( board, &priv->tms9914_priv );
}
void agilent_82350b_request_system_control( gpib_board_t *board, int request_control )
{
	agilent_82350b_private_t *a_priv = board->private_data;
	
	if(request_control)
	{
		a_priv->card_mode_bits |= CM_SYSTEM_CONTROLLER_BIT;
		writeb(IC_SYSTEM_CONTROLLER_BIT, a_priv->gpib_base + INTERNAL_CONFIG_REG); 
	}else
	{
		a_priv->card_mode_bits &= ~CM_SYSTEM_CONTROLLER_BIT;
		writeb(0, a_priv->gpib_base + INTERNAL_CONFIG_REG); 
	}
	writeb(a_priv->card_mode_bits, a_priv->gpib_base + CARD_MODE_REG); 
	tms9914_request_system_control(board, &a_priv->tms9914_priv, request_control);
}
void agilent_82350b_interface_clear( gpib_board_t *board, int assert )
{
	agilent_82350b_private_t *priv = board->private_data;
	tms9914_interface_clear( board, &priv->tms9914_priv, assert );
}
void agilent_82350b_remote_enable( gpib_board_t *board, int enable )
{
	agilent_82350b_private_t *priv = board->private_data;
	tms9914_remote_enable( board, &priv->tms9914_priv, enable );
}
void agilent_82350b_enable_eos( gpib_board_t *board, uint8_t eos_byte, int compare_8_bits )
{
	agilent_82350b_private_t *priv = board->private_data;
	tms9914_enable_eos( board, &priv->tms9914_priv, eos_byte, compare_8_bits );
}
void agilent_82350b_disable_eos( gpib_board_t *board )
{
	agilent_82350b_private_t *priv = board->private_data;
	tms9914_disable_eos( board, &priv->tms9914_priv );
}
unsigned int agilent_82350b_update_status( gpib_board_t *board, unsigned int clear_mask )
{
	agilent_82350b_private_t *priv = board->private_data;
	return tms9914_update_status( board, &priv->tms9914_priv, clear_mask );
}
void agilent_82350b_primary_address( gpib_board_t *board, unsigned int address )
{
	agilent_82350b_private_t *priv = board->private_data;
	tms9914_primary_address( board, &priv->tms9914_priv, address );
}
void agilent_82350b_secondary_address( gpib_board_t *board, unsigned int address, int enable )
{
	agilent_82350b_private_t *priv = board->private_data;
	tms9914_secondary_address( board, &priv->tms9914_priv, address, enable );
}
int agilent_82350b_parallel_poll( gpib_board_t *board, uint8_t *result )
{
	agilent_82350b_private_t *priv = board->private_data;
	return tms9914_parallel_poll( board, &priv->tms9914_priv, result );
}
void agilent_82350b_parallel_poll_configure( gpib_board_t *board, uint8_t config )
{
	agilent_82350b_private_t *priv = board->private_data;
	tms9914_parallel_poll_configure( board, &priv->tms9914_priv, config );
}
void agilent_82350b_parallel_poll_response( gpib_board_t *board, int ist )
{
	agilent_82350b_private_t *priv = board->private_data;
	tms9914_parallel_poll_response( board, &priv->tms9914_priv, ist );
}
void agilent_82350b_serial_poll_response( gpib_board_t *board, uint8_t status )
{
	agilent_82350b_private_t *priv = board->private_data;
	tms9914_serial_poll_response( board, &priv->tms9914_priv, status );
}
uint8_t agilent_82350b_serial_poll_status( gpib_board_t *board )
{
	agilent_82350b_private_t *priv = board->private_data;
	return tms9914_serial_poll_status( board, &priv->tms9914_priv );
}
int agilent_82350b_line_status( const gpib_board_t *board )
{
	agilent_82350b_private_t *priv = board->private_data;
	return tms9914_line_status( board, &priv->tms9914_priv );
}
unsigned int agilent_82350b_t1_delay( gpib_board_t *board, unsigned int nanosec )
{
	agilent_82350b_private_t *a_priv = board->private_data;
	static const int nanosec_per_clock = 30;
	unsigned value = (nanosec + nanosec_per_clock - 1) / nanosec_per_clock;
	if(value > 0xff) value = 0xff;
	writeb(value, a_priv->gpib_base + T1_DELAY_REG);
	return value * nanosec_per_clock;
//	return tms9914_t1_delay( board, &priv->tms9914_priv, nano_sec );
}
void agilent_82350b_return_to_local( gpib_board_t *board )
{
	agilent_82350b_private_t *priv = board->private_data;
	tms9914_return_to_local( board, &priv->tms9914_priv );
}

gpib_interface_t agilent_82350b_interface =
{
	name: "agilent_82350b",
	attach: agilent_82350b_attach,
	detach: agilent_82350b_detach,
	read: agilent_82350b_read,
	write: agilent_82350b_write,
	command: agilent_82350b_command,
	request_system_control: agilent_82350b_request_system_control,
	take_control: agilent_82350b_take_control,
	go_to_standby: agilent_82350b_go_to_standby,
	interface_clear: agilent_82350b_interface_clear,
	remote_enable: agilent_82350b_remote_enable,
	enable_eos: agilent_82350b_enable_eos,
	disable_eos: agilent_82350b_disable_eos,
	parallel_poll: agilent_82350b_parallel_poll,
	parallel_poll_configure: agilent_82350b_parallel_poll_configure,
	parallel_poll_response: agilent_82350b_parallel_poll_response,
	line_status: agilent_82350b_line_status,
	update_status: agilent_82350b_update_status,
	primary_address: agilent_82350b_primary_address,
	secondary_address: agilent_82350b_secondary_address,
	serial_poll_response: agilent_82350b_serial_poll_response,
	t1_delay: agilent_82350b_t1_delay,
	return_to_local: agilent_82350b_return_to_local,
	provider_module: &__this_module,
};

int agilent_82350b_allocate_private( gpib_board_t *board )
{
	board->private_data = kmalloc(sizeof(agilent_82350b_private_t), GFP_KERNEL);
	if(board->private_data == NULL)
		return -ENOMEM;
	memset(board->private_data, 0, sizeof(agilent_82350b_private_t));
	return 0;
}

void agilent_82350b_free_private( gpib_board_t *board )
{
	if(board->private_data)
	{
		kfree(board->private_data);
		board->private_data = NULL;
	}
}

static inline unsigned int tms9914_to_agilent_82350b_offset( unsigned int register_num )
{
	return 0x3ff8 + register_num;
}

int agilent_82350b_attach( gpib_board_t *board )
{
	agilent_82350b_private_t *a_priv;
	tms9914_private_t *tms_priv;

	board->status = 0;

	if(agilent_82350b_allocate_private(board))
		return -ENOMEM;
	a_priv = board->private_data;
	tms_priv = &a_priv->tms9914_priv;
	tms_priv->read_byte = tms9914_iomem_read_byte;
	tms_priv->write_byte = tms9914_iomem_write_byte;
	tms_priv->offset = 1;

	// find board
	a_priv->pci_device = gpib_pci_find_device(board, PCI_VENDOR_ID_AGILENT,
		PCI_DEVICE_ID_82350B, NULL);
	if(a_priv->pci_device == NULL)
	{
		printk("gpib: no 82350B board found\n");
		return -ENODEV;
	}
	if(pci_request_regions(a_priv->pci_device, "agilent_82350b"))
		return -EIO;
	a_priv->gpib_base = (unsigned long) ioremap(pci_resource_start(a_priv->pci_device, GPIB_REGION),
		pci_resource_len(a_priv->pci_device, GPIB_REGION));
	printk("%s: gpib base address remapped to 0x%lx\n", __FUNCTION__, a_priv->gpib_base );
	tms_priv->iobase = a_priv->gpib_base + TMS9914_BASE_REG;
	a_priv->sram_base = (unsigned long) ioremap(pci_resource_start(a_priv->pci_device, SRAM_REGION),
		pci_resource_len(a_priv->pci_device, SRAM_REGION));
	printk("%s: sram base address remapped to 0x%lx\n", __FUNCTION__, a_priv->sram_base );
	a_priv->misc_base = (unsigned long) ioremap(pci_resource_start(a_priv->pci_device, MISC_REGION),
		pci_resource_len(a_priv->pci_device, MISC_REGION));
	printk("%s: misc base address remapped to 0x%lx\n", __FUNCTION__, a_priv->misc_base );

	if(request_irq(a_priv->pci_device->irq, agilent_82350b_interrupt, SA_SHIRQ, "agilent_82350b", board))
	{
		printk("gpib: can't request IRQ %d\n", a_priv->pci_device->irq);
		return -EIO;
	}
	a_priv->irq = a_priv->pci_device->irq;
	printk( "agilent_82350b: IRQ %d\n", a_priv->irq );
	if(pci_enable_device(a_priv->pci_device))
	{
		printk("error enabling pci device\n");
		return -EIO;
	}
	
	writeb(0, a_priv->gpib_base + SRAM_ACCESS_CONTROL_REG); 
	a_priv->card_mode_bits = ENABLE_PCI_IRQ_BIT;
	writeb(a_priv->card_mode_bits, a_priv->gpib_base + CARD_MODE_REG); 
	writeb(ENABLE_TMS9914_INTERRUPTS_BIT, a_priv->gpib_base + INTERRUPT_ENABLE_REG); 
	board->t1_nano_sec = agilent_82350b_t1_delay(board, 2000);
	
	tms9914_board_reset(tms_priv);

	tms9914_online( board, tms_priv );

	return 0;
}

void agilent_82350b_detach(gpib_board_t *board)
{
	agilent_82350b_private_t *a_priv = board->private_data;
	tms9914_private_t *tms_priv;

	if(a_priv)
	{
		tms_priv = &a_priv->tms9914_priv;
		if(a_priv->irq)
		{
			free_irq(a_priv->irq, board);
		}
		if(a_priv->gpib_base)
		{
			tms9914_board_reset(tms_priv);
			iounmap((void *)a_priv->misc_base);
			iounmap((void *)a_priv->sram_base);
			iounmap((void *)a_priv->gpib_base);
			pci_release_regions(a_priv->pci_device);
		}
	}
	agilent_82350b_free_private( board );
}


static int agilent_82350b_init_module( void )
{
	gpib_register_driver(&agilent_82350b_interface);
	return 0;
}

static void agilent_82350b_exit_module( void )
{
	gpib_unregister_driver(&agilent_82350b_interface);
}

module_init( agilent_82350b_init_module );
module_exit( agilent_82350b_exit_module );

/*
 * GPIB interrupt service routines
 */

irqreturn_t agilent_82350b_interrupt(int irq, void *arg, struct pt_regs *registerp)
{
	int status1, status2;
	gpib_board_t *board = arg;
	agilent_82350b_private_t *priv = board->private_data;
	unsigned long flags;
	irqreturn_t retval;
	
	spin_lock_irqsave( &board->spinlock, flags );
	status1 = read_byte( &priv->tms9914_priv, ISR0);
	status2 = read_byte( &priv->tms9914_priv, ISR1);
	retval = tms9914_interrupt_have_status(board, &priv->tms9914_priv, status1, status2);
	spin_unlock_irqrestore( &board->spinlock, flags );
	return retval;
}

