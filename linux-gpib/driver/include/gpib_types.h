/***************************************************************************
                                gpib_types.h
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

#ifndef _GPIB_TYPES_H
#define _GPIB_TYPES_H

#ifndef __KERNEL__
#include <stdint.h>
#endif

typedef struct ibio_op {
	uint8_t 		*io_vbuf;	/* virtual buffer address	*/
	unsigned long		io_pbuf;	/* physical buffer address	*/
	unsigned long	io_cnt;		/* transfer count		*/
	int		io_flags;	/* direction flags, etc.	*/
	uint8_t		io_ccfunc;	/* carry-cycle function		*/
} ibio_op_t;

typedef struct {
	int		ib_cnt;		/* I/O count  (rd, wrt, etc)	*/
	int		ib_arg;		/* other argument value 	*/
	int		ib_ret;		/* general purpose return value	*/
	int		ib_ibsta;	/* returned status vector	*/
	int		ib_iberr;	/* returned error code (if ERR)	*/
	int		ib_ibcnt;	/* returned I/O count		*/
	uint8_t            *ib_buf;
} ibarg_t;

#ifdef __KERNEL__
/* gpib_interface_t defines the interface
 * between the board-specific details dealt with in the drivers
 * and generic interface provided by gpib-common.
 * This really should be in a different header file.
 */
#include <linux/wait.h>

typedef struct gpib_interface_struct gpib_interface_t;
typedef struct gpib_device_struct gpib_device_t;

struct gpib_interface_struct
{
	// list_head so we can make a linked list of drivers
	struct list_head list;
	// name of board
	char *name;
	/* attach() initializes board and allocates resources */
	int (*attach)(gpib_device_t *device);
	/* detach() shuts down board and frees resources */
	void (*detach)(gpib_device_t *device);
	/* read() should read at most 'length' bytes from the bus into
	 * 'buffer'.  It should return when it fills the buffer or
	 * encounters an END (EOI and or EOS if appropriate).  It should set 'end'
	 * to be nonzero if the read was terminated by an END, otherwise 'end'
	 * should be zero.
	 * Ultimately, this will be changed into or replaced by an asynchronous
	 * read.  Positive return value is number of bytes read, negative
	 * return indicates error.
	 */
	ssize_t (*read)(gpib_device_t *device, uint8_t *buffer, size_t length, int
 *end);
	/* write() should write 'length' bytes from buffer to the bus.
	 * If the boolean value send_eoi is nonzero, then EOI should
	 * be sent along with the last byte.  Returns number of bytes
	 * written or negative value on error.
	 */
	ssize_t (*write)(gpib_device_t *device, uint8_t *buffer, size_t length, int
 send_eoi);
	/* command() writes the command bytes in 'buffer' to the bus
	 * Returns number of bytes written or negative value on error.
	 */
	ssize_t (*command)(gpib_device_t *device, uint8_t *buffer, size_t length);
	/* Take control (assert ATN).  If 'asyncronous' is nonzero, take
	 * control asyncronously (assert ATN immediately without waiting
	 * for other processes to complete first).  Should not return
	 * until board becomes controller in charge.  Returns zero no success,
	 * nonzero on error.
	 */
	int (*take_control)(gpib_device_t *device, int asyncronous);
	/* De-assert ATN.  Returns zero on success, nonzer on error.
	 */
	int (*go_to_standby)(gpib_device_t *device);
	/* Asserts or de-asserts 'interface clear' (IFC) depending on
	 * boolean value of 'assert'
	 */
	void (*interface_clear)(gpib_device_t *device, int assert);
	/* Sends remote enable command if 'enable' is nonzero, disables remote mode
	 * if 'enable' is zero
	 */
	void (*remote_enable)(gpib_device_t *device, int enable);
	/* enable END for reads, when byte 'eos' is received.  If
	 * 'compare_8_bits' is nonzero, then all 8 bits are compared
	 * with the eos bytes.  Otherwise only the 7 least significant
	 * bits are compared. */
	void (*enable_eos)(gpib_device_t *device, uint8_t eos, int compare_8_bits);
	/* disable END on eos byte (END on EOI only)*/
	void (*disable_eos)(gpib_device_t *device);
	/* TODO: conduct parallel poll */
	int (*parallel_poll)(gpib_device_t *device, uint8_t *result);
	/* Returns current status of the bus lines.  Should be set to
	 * NULL if your board does not have the ability to query the
	 * state of the bus lines. */
	int (*line_status)(gpib_device_t *device);
	/* updates and returns the board's current status.
	 * The meaning of the bits are specified in gpib_user.h
	 * in the IBSTA section.  The driver does not need to
	 * worry about setting the CMPL, END, TIMO, or ERR bits.
	 */
	unsigned int (*update_status)(gpib_device_t *device);
	/* Sets primary address 0-30 for gpib interface card.
	 */
	void (*primary_address)(gpib_device_t *device, unsigned int address);
	/* Sets and enables, or disables secondary address 0-30 for gpib interface
 card.
	 */
	void (*secondary_address)(gpib_device_t *device, unsigned int address, int
 enable);
	/* Sets the byte the board should send in response to a serial poll.  Returns
	 * zero on success.  Function should also request service if appropriate.
	 */
	int (*serial_poll_response)(gpib_device_t *device, uint8_t status);
};

/* One gpib_device_t is allocated for each physical board in the computer.
 * It provides storage for variables local to each board, and interface
 * functions for performing operations on the board */
struct gpib_device_struct
{
	/* functions used by this device */
	gpib_interface_t *interface;
	/* Used to hold the board's current status (see update_status() above)
	 */
	volatile unsigned int status;
	/* Driver should only sleep on this wait queue.  It is special in that the
	 * core will wake this queue and set the TIMO bit in 'status' when the
	 * watchdog timer times out.
	 */
	wait_queue_head_t wait;
	/* Lock that only allows one process to access this device at a time */
	struct semaphore mutex;
	/* IO base address to use for non-pnp cards (set by core, driver should make local copy) */
	unsigned long ibbase;
	/* IRQ to use for non-pnp cards (set by core, driver should make local copy) */
	unsigned int ibirq;
	/* dma channel to use for non-pnp cards (set by core, driver should make local copy) */
	unsigned int ibdma;
	/* 'private_data' can be used as seen fit by the driver to
	 * store additional variables for this board */
	void *private_data;
};

#endif	// __KERNEL__

#endif	// _GPIB_TYPES_H
