/***************************************************************************
                              nec7210/tnt4882.h
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

#ifndef _TNT4882_H
#define _TNT4882_H

#include <nec7210.h>
#include <gpibP.h>
#include "mite.h"

enum
{
	PCI_DEVICE_ID_NI_GPIB = 0xc801,
};

// struct which defines private_data for tnt4882 devices
typedef struct
{
	nec7210_private_t nec7210_priv;
	struct mite_struct *mite;
	unsigned int irq;
} tnt4882_private_t;

// interfaces
extern gpib_interface_t ni_isa_interface;

// interface functions
ssize_t tnt4882_read(gpib_device_t *device, uint8_t *buffer, size_t length, int
 *end);
ssize_t tnt4882_write(gpib_device_t *device, uint8_t *buffer, size_t length, int
 send_eoi);
ssize_t tnt4882_command(gpib_device_t *device, uint8_t *buffer, size_t length);
int tnt4882_take_control(gpib_device_t *device, int synchronous);
int tnt4882_go_to_standby(gpib_device_t *device);
void tnt4882_interface_clear(gpib_device_t *device, int assert);
void tnt4882_remote_enable(gpib_device_t *device, int enable);
void tnt4882_enable_eos(gpib_device_t *device, uint8_t eos_byte, int
 compare_8_bits);
void tnt4882_disable_eos(gpib_device_t *device);
unsigned int tnt4882_update_status(gpib_device_t *device);
void tnt4882_primary_address(gpib_device_t *device, unsigned int address);
void tnt4882_secondary_address(gpib_device_t *device, unsigned int address, int
 enable);
int tnt4882_parallel_poll(gpib_device_t *device, uint8_t *result);
int tnt4882_serial_poll_response(gpib_device_t *device, uint8_t status);

// interrupt service routines
void tnt4882_interrupt(int irq, void *arg, struct pt_regs *registerp);

// utility functions
int tnt4882_allocate_private(gpib_device_t *device);
void tnt4882_free_private(gpib_device_t *device);

// register offset for nec7210 compatible registers
static const int atgpib_reg_offset = 2;

// number of ioports used
static const int atgpib_iosize = 32;

// tnt4882 specific registers and bits
#define GPIBC	0x1
#define GPIBD	0x3
#define KEYPRT	0x5
#define INTRT 0x7
/* TURBO488...                          */
#define CFG	0x10
#define IMR3	0x12
#define CNTL	0x14
#define CNTH	0x16
#define FIFO	0x18	// fifo can be single 16 bit register or two 8 bit
#define FIFOB	0x18
#define FIFOA	0x19
#define CCRG	0x1a	// carry cycle register
#define CMDR	0x1c	// command register
#define TIMER	0x1e	// timer register

#define STS1	0x10		/* T488 Status Register 1 */
#define STS2	0x1c	        /* T488 Status Register 2 */
#define ISR3	0x1a		/* T488 Interrupt Status Register 3 */
#define DMA_EN	0x5		/* DMA Enable Register (Key Port) */

/*============================================================*/

/* TURBO-488 registers bit definitions */

/* STS1 -- Status Register 1 (read only) */
#define S_DONE          (0x80)	/* DMA done                           */
#define S_SC            (0x40)	/* is system contoller                */
#define S_IN            (0x20)	/* DMA in (to memory)                 */
#define S_DRQ           (0x10)	/* DRQ line (for diagnostics)         */
#define S_STOP          (0x08)	/* DMA stopped                        */
#define S_NDAV          (0x04)	/* inverse of DAV                     */
#define S_HALT          (0x02)	/* status of transfer machine         */
#define S_GSYNC         (0x01)	/* indicates if GPIB is in sync w I/O */

/* CFG -- Configuration Register (write only) */
#define	C_CMD	        (1<<7)	/* FIFO 'bcmd' in progress            */
#define	C_TLCH	     (1<<6)	/* halt DMA on TLC interrupt          */
#define	C_IN	        (1<<5)	/* DMA is a GPIB read                 */
#define	C_A_B	        (1<<4)	/* fifo order 1=motorola, 0=intel     */
#define	C_CCEN	     (1<<3)	/* enable carry cycle                 */
#define	C_TMOE	     (1<<2)	/* enable CPU bus time limit          */
#define	C_T_B	        (1<<1)  	/* tmot reg is: 1=125ns clocks,       */
						/* 0=num bytes                        */
#define	C_B16	        (1<<0)  	/* 1=FIFO is 16-bit register, 0=8-bit */

/* ISR3 -- Interrupt Status Register (read only) */
#define	HR_INTR	        (1<<7)	/* 1=board is interrupting	*/
#define	HR_SRQI_CIC      (1<<5)	/* SRQ asserted and we are CIC	*/
#define	HR_STOP          (1<<4)	/* fifo empty or STOP command	*/
						/* issued			*/
#define	HR_NFF	        (1<<3)	/* NOT full fifo		*/
#define	HR_NEF	        (1<<2)	/* NOT empty fifo		*/
#define	HR_TLCI	        (1<<1)	/* TLC interrupt asserted	*/
#define	HR_DONE          (1<<0)	/* DMA done			*/

/* CMDR -- Command Register */
#define	GO		(1<<2)	/* start DMA 			*/
#define	STOP		(1<<3)	/* stop DMA 			*/
#define	RSTFIFO		(1<<4)	/* reset the FIFO 		*/
#define SFTRST		0x22	/* issue a software reset 	*/

/* STS2 -- Status Register 2 */
#define AFFN		(1<<3)	/* "A full FIFO NOT"  (0=FIFO full)  */
#define AEFN		(1<<2)	/* "A empty FIFO NOT" (0=FIFO empty) */
#define BFFN		(1<<1)	/* "B full FIFO NOT"  (0=FIFO full)  */
#define BEFN		(1<<0)	/* "B empty FIFO NOT" (0=FIFO empty) */


#endif	// _TNT4882_H
