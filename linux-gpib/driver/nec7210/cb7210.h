/***************************************************************************
                              nec7210/cb7210.h
                             -------------------

    begin                : Jan 2002
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

#ifndef _CB7210_H
#define _CB7210_H

#include "amccs5933.h"

#if DMAOP && defined(CBI_PCI)
#error pci-gpib does not support ISA DMA, run make config again
#endif

#define PCI_DEVICE_ID_CBOARDS_PCI_GPIB 0x6

// struct which defines private_data for cb7210 drivers
typedef struct
{
	nec7210_private_t nec7210_priv;
	struct pci_dev *pci_device;
	// base address of amccs5933 pci chip
	unsigned long amcc_iobase;
	unsigned int irq;
} cb7210_private_t;

// drivers
extern gpib_driver_t cb_pci_driver;
extern gpib_driver_t cb_pcmcia_driver;

// interrupt service routines
void cb_pci_interrupt(int irq, void *arg, struct pt_regs *registerp);
void cb7210_interrupt(int irq, void *arg, struct pt_regs *registerp);

// interface functions
ssize_t cb7210_read(gpib_driver_t *driver, uint8_t *buffer, size_t length, int *end);
ssize_t cb7210_write(gpib_driver_t *driver, uint8_t *buffer, size_t length, int send_eoi);
ssize_t cb7210_command(gpib_driver_t *driver, uint8_t *buffer, size_t length);
int cb7210_take_control(gpib_driver_t *driver, int synchronous);
int cb7210_go_to_standby(gpib_driver_t *driver);
void cb7210_interface_clear(gpib_driver_t *driver, int assert);
void cb7210_remote_enable(gpib_driver_t *driver, int enable);
void cb7210_enable_eos(gpib_driver_t *driver, uint8_t eos_byte, int compare_8_bits);
void cb7210_disable_eos(gpib_driver_t *driver);
unsigned int cb7210_update_status(gpib_driver_t *driver);
void cb7210_primary_address(gpib_driver_t *driver, unsigned int address);
void cb7210_secondary_address(gpib_driver_t *driver, unsigned int address, int enable);
int cb7210_parallel_poll(gpib_driver_t *driver, uint8_t *result);
int cb7210_serial_poll_response(gpib_driver_t *driver, uint8_t status);

// utility functions
int cb7210_allocate_private(gpib_driver_t *driver);
void cb7210_free_private(gpib_driver_t *driver);

// pcmcia init/cleanup
int pcmcia_init_module(void);
void pcmcia_cleanup_module(void);

// pci-gpib register offset
static const int cb7210_reg_offset = 1;

// cb7210 specific registers and bits

#define HS_MODE	(0x8 * NEC7210_REG_OFFSET)	/* HS_MODE register */
#define HS_INT_LEVEL	(0x9 * NEC7210_REG_OFFSET)	/* HS_INT_LEVEL register */

#define HS_STATUS	(0x8 * NEC7210_REG_OFFSET)	/* HS_STATUS register */

/* CBI 488.2 HS control */

/* when both bit 0 and 1 are set, it
 *   1 clears the transmit state machine to an initial condition
 *   2 clears any residual interrupts left latched on cbi488.2
 *   3 resets all control bits in HS_MODE to zero
 *   4 enables TX empty interrupts
 * when both bit 0 and 1 are zero, then the high speed mode is disabled
 */
#define HS_TX_ENABLE     (1<<0)
#define HS_RX_ENABLE     (1<<1)
#define HS_HF_INT_EN     (1<<3)
#define HS_CLR_SRQ_INT   (1<<4)
#define HS_CLR_EOI_INT   (1<<5) /* RX enabled */
#define HS_CLR_EMPTY_INT (1<<5) /* TX enabled */
#define HS_CLR_HF_INT    (1<<6)
#define HS_SYS_CONTROL   (1<<7)

/* CBI 488.2 status */

#define HS_FIFO_FULL        (1<<0)
#define HS_HALF_FULL        (1<<1)
#define HS_SRQ_INT          (1<<2)
#define HS_EOI_INT          (1<<3)
#define HS_TX_MSB_NOT_EMPTY (1<<4)
#define HS_RX_MSB_NOT_EMPTY (1<<5)
#define HS_TX_LSB_NOT_EMPTY (1<<6)
#define HS_RX_LSB_NOT_EMPTY (1<<7)

/* CBI488.2 hs_int_level register */

#define HS_RESET7210    (1<<7)
#define AUX_HISPEED     0x41
#define AUX_LOSPEED     0x40


#endif _CB7210_H
