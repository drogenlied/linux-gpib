/***************************************************************************
                          nec7210/pc2_init.c  -  description
                             -------------------
 initialization for pc2 and pc2a compatible boards

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

#include "pc2.h"
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <asm/dma.h>
#include <linux/pci.h>
#include <linux/string.h>

#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

int pc2_attach(gpib_board_t *board);
int pc2a_attach(gpib_board_t *board);

void pc2_detach(gpib_board_t *board);
void pc2a_detach(gpib_board_t *board);

// wrappers for interface functions
ssize_t pc2_read(gpib_board_t *board, uint8_t *buffer, size_t length, int *end)
{
	pc2_private_t *priv = board->private_data;
	return nec7210_read(board, &priv->nec7210_priv, buffer, length, end);
}
ssize_t pc2_write(gpib_board_t *board, uint8_t *buffer, size_t length, int send_eoi)
{
	pc2_private_t *priv = board->private_data;
	return nec7210_write(board, &priv->nec7210_priv, buffer, length, send_eoi);
}
ssize_t pc2_command(gpib_board_t *board, uint8_t *buffer, size_t length)
{
	pc2_private_t *priv = board->private_data;
	return nec7210_command(board, &priv->nec7210_priv, buffer, length);
}
int pc2_take_control(gpib_board_t *board, int synchronous)
{
	pc2_private_t *priv = board->private_data;
	return nec7210_take_control(board, &priv->nec7210_priv, synchronous);
}
int pc2_go_to_standby(gpib_board_t *board)
{
	pc2_private_t *priv = board->private_data;
	return nec7210_go_to_standby(board, &priv->nec7210_priv);
}
void pc2_interface_clear(gpib_board_t *board, int assert)
{
	pc2_private_t *priv = board->private_data;
	nec7210_interface_clear(board, &priv->nec7210_priv, assert);
}
void pc2_remote_enable(gpib_board_t *board, int enable)
{
	pc2_private_t *priv = board->private_data;
	nec7210_remote_enable(board, &priv->nec7210_priv, enable);
}
void pc2_enable_eos(gpib_board_t *board, uint8_t eos_byte, int compare_8_bits)
{
	pc2_private_t *priv = board->private_data;
	nec7210_enable_eos(board, &priv->nec7210_priv, eos_byte, compare_8_bits);
}
void pc2_disable_eos(gpib_board_t *board)
{
	pc2_private_t *priv = board->private_data;
	nec7210_disable_eos(board, &priv->nec7210_priv);
}
unsigned int pc2_update_status(gpib_board_t *board)
{
	pc2_private_t *priv = board->private_data;
	return nec7210_update_status(board, &priv->nec7210_priv);
}
void pc2_primary_address(gpib_board_t *board, unsigned int address)
{
	pc2_private_t *priv = board->private_data;
	nec7210_primary_address(board, &priv->nec7210_priv, address);
}
void pc2_secondary_address(gpib_board_t *board, unsigned int address, int enable)
{
	pc2_private_t *priv = board->private_data;
	nec7210_secondary_address(board, &priv->nec7210_priv, address, enable);
}
int pc2_parallel_poll(gpib_board_t *board, uint8_t *result)
{
	pc2_private_t *priv = board->private_data;
	return nec7210_parallel_poll(board, &priv->nec7210_priv, result);
}
int pc2_serial_poll_response(gpib_board_t *board, uint8_t status)
{
	pc2_private_t *priv = board->private_data;
	return nec7210_serial_poll_response(board, &priv->nec7210_priv, status);
}

gpib_interface_t pc2_interface =
{
	name:	"pcII",
	attach:	pc2_attach,
	detach:	pc2_detach,
	read:	pc2_read,
	write:	pc2_write,
	command:	pc2_command,
	take_control:	pc2_take_control,
	go_to_standby:	pc2_go_to_standby,
	interface_clear:	pc2_interface_clear,
	remote_enable:	pc2_remote_enable,
	enable_eos:	pc2_enable_eos,
	disable_eos:	pc2_disable_eos,
	parallel_poll:	pc2_parallel_poll,
	line_status:	NULL,
	update_status:	pc2_update_status,
	primary_address:	pc2_primary_address,
	secondary_address:	pc2_secondary_address,
	serial_poll_response:	pc2_serial_poll_response,
};

gpib_interface_t pc2a_interface =
{
	name:	"pcIIa",
	attach:	pc2a_attach,
	detach:	pc2a_detach,
	read:	pc2_read,
	write:	pc2_write,
	command:	pc2_command,
	take_control:	pc2_take_control,
	go_to_standby:	pc2_go_to_standby,
	interface_clear:	pc2_interface_clear,
	remote_enable:	pc2_remote_enable,
	enable_eos:	pc2_enable_eos,
	disable_eos:	pc2_disable_eos,
	parallel_poll:	pc2_parallel_poll,
	line_status:	NULL,
	update_status:	pc2_update_status,
	primary_address:	pc2_primary_address,
	secondary_address:	pc2_secondary_address,
	serial_poll_response:	pc2_serial_poll_response,
};

static int allocate_private(gpib_board_t *board)
{
	board->private_data = kmalloc(sizeof(pc2_private_t), GFP_KERNEL);
	if(board->private_data == NULL)
		return -1;
	memset(board->private_data, 0, sizeof(pc2_private_t));
	return 0;
}

static void free_private(gpib_board_t *board)
{
	if(board->private_data)
	{
		kfree(board->private_data);
		board->private_data = NULL;
	}
}

int pc2_generic_attach(gpib_board_t *board)
{
	pc2_private_t *pc2_priv;
	nec7210_private_t *nec_priv;

	board->status = 0;
	if(allocate_private(board))
		return -ENOMEM;
	pc2_priv = board->private_data;
	nec_priv = &pc2_priv->nec7210_priv;
	nec_priv->read_byte = nec7210_ioport_read_byte;
	nec_priv->write_byte = nec7210_ioport_write_byte;

#if DMAOP
	nec_priv->dma_buffer_length = 0x1000;
	nec_priv->dma_buffer = pci_alloc_consistent(NULL, nec_priv->dma_buffer_length,
		&nec_priv->dma_buffer_addr);
	if(nec_priv->dma_buffer == NULL)
		return -ENOMEM;

	// request isa dma channel
	if( request_dma( board->ibdma, "pc2" ) )
	{
		printk("gpib: can't request DMA %d\n", board->ibdma);
		return -1;
	}
	nec_priv->dma_channel = board->ibdma;
#endif

	return 0;
}

void pc2_init(nec7210_private_t *nec_priv)
{
	nec7210_board_reset(nec_priv);

	// enable interrupts
	nec_priv->imr1_bits = HR_ERRIE | HR_DECIE | HR_ENDIE |
		HR_DETIE | HR_APTIE | HR_CPTIE | HR_DOIE | HR_DIIE;
	nec_priv->imr2_bits = IMR2_ENABLE_INTR_MASK;
	write_byte(nec_priv, nec_priv->imr1_bits, IMR1);
	write_byte(nec_priv, nec_priv->imr2_bits, IMR2);

	write_byte(nec_priv, AUX_PON, AUXMR);
}

int pc2_attach(gpib_board_t *board)
{
	int isr_flags = 0;
	pc2_private_t *pc2_priv;
	nec7210_private_t *nec_priv;
	int retval;

	retval = pc2_generic_attach(board);
	if(retval) return retval;

	pc2_priv = board->private_data;
	nec_priv = &pc2_priv->nec7210_priv;
	nec_priv->offset = pc2_reg_offset;

	if(request_region(board->ibbase, pc2_iosize, "pc2"));
	{
		printk("gpib: ioports are already in use");
		return -1;
	}
	nec_priv->iobase = board->ibbase;

	// install interrupt handler
	if( request_irq(board->ibirq, pc2_interrupt, isr_flags, "pc2", board))
	{
		printk("gpib: can't request IRQ %d\n", board->ibirq);
		return -1;
	}
	pc2_priv->irq = board->ibirq;

	pc2_init(nec_priv);

	return 0;
}

void pc2_detach(gpib_board_t *board)
{
	pc2_private_t *pc2_priv = board->private_data;
	nec7210_private_t *nec_priv;

	if(pc2_priv)
	{
		nec_priv = &pc2_priv->nec7210_priv;
		if(nec_priv->dma_channel)
		{
			free_dma(nec_priv->dma_channel);
		}
		if(pc2_priv->irq)
		{
			free_irq(pc2_priv->irq, board);
		}
		if(nec_priv->iobase)
		{
			nec7210_board_reset(nec_priv);
			release_region(nec_priv->iobase, pc2_iosize);
		}
		if(nec_priv->dma_buffer)
		{
			pci_free_consistent(NULL, nec_priv->dma_buffer_length, nec_priv->dma_buffer,
				nec_priv->dma_buffer_addr);
			nec_priv->dma_buffer = NULL;
		}
	}
	free_private(board);
}

int pc2a_attach(gpib_board_t *board)
{
	unsigned int i, err;
	int isr_flags = 0;
	pc2_private_t *pc2_priv;
	nec7210_private_t *nec_priv;
	int retval;

	retval = pc2_generic_attach(board);
	if(retval) return retval;

	pc2_priv = board->private_data;
	nec_priv = &pc2_priv->nec7210_priv;
	nec_priv->offset = pc2a_reg_offset;

	switch( board->ibbase ){

		case 0x02e1:
		case 0x22e1:
		case 0x42e1:
		case 0x62e1:
			break;
		default:
			printk("PCIIa base range invalid, must be one of [0246]2e1 is %lx \n", board->ibbase);
			return -1;
			break;
	}

	if(board->ibirq < 2 || board->ibirq > 7 )
	{
		printk("Illegal Interrupt Level \n");
		return -1;
	}

	err = 0;
	for(i = 0; i < nec7210_num_registers; i++)
	{
		if(check_region(board->ibbase + i * pc2a_reg_offset, 1))
			err++;
	}
	if(check_region(pc2a_clear_intr_iobase + board->ibirq, 1))
	{
		err++;
	}
	if(err)
	{
		printk("gpib: ioports are already in use");
		return -1;
	}
	for(i = 0; i < nec7210_num_registers; i++)
	{
		request_region(board->ibbase + i * pc2a_reg_offset, 1, "pc2a");
	}
	nec_priv->iobase = board->ibbase;
	request_region(pc2a_clear_intr_iobase + board->ibirq, 1, "pc2a");
	pc2_priv->clear_intr_addr = pc2a_clear_intr_iobase + board->ibirq;

	if(request_irq(board->ibirq, pc2a_interrupt, isr_flags, "pc2a", board))
	{
		printk("gpib: can't request IRQ %d\n", board->ibirq);
		return -1;
	}
	pc2_priv->irq = board->ibirq;

	pc2_init(nec_priv);

	// make sure interrupt is clear
	outb(0xff , CLEAR_INTR_REG(pc2_priv->irq));

	return 0;
}

void pc2a_detach(gpib_board_t *board)
{
	int i;
	pc2_private_t *pc2_priv = board->private_data;
	nec7210_private_t *nec_priv;

	if(pc2_priv)
	{
		nec_priv = &pc2_priv->nec7210_priv;
		if(nec_priv->dma_channel)
		{
			free_dma(nec_priv->dma_channel);
		}
		if(pc2_priv->irq)
		{
			free_irq(pc2_priv->irq, board);
		}
		if(nec_priv->iobase)
		{
			nec7210_board_reset(nec_priv);
			for(i = 0; i < nec7210_num_registers; i++)
				release_region(nec_priv->iobase + i * pc2a_reg_offset, 1);
		}
		if(pc2_priv->clear_intr_addr)
			release_region(pc2_priv->clear_intr_addr, 1);
		if(nec_priv->dma_buffer)
		{
			pci_free_consistent(NULL, nec_priv->dma_buffer_length, nec_priv->dma_buffer,
				nec_priv->dma_buffer_addr);
			nec_priv->dma_buffer = NULL;
		}
	}
	free_private(board);
}

int init_module(void)
{
	EXPORT_NO_SYMBOLS;

	INIT_LIST_HEAD(&pc2_interface.list);
	INIT_LIST_HEAD(&pc2a_interface.list);

	gpib_register_driver(&pc2_interface);
	gpib_register_driver(&pc2a_interface);

	return 0;
}

void cleanup_module(void)
{
	gpib_unregister_driver(&pc2_interface);
	gpib_unregister_driver(&pc2a_interface);
}









