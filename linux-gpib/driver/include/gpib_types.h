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

typedef unsigned char	*faddr_t;

typedef struct ibio_op {
	faddr_t		io_vbuf;	/* virtual buffer address	*/
	uint32_t		io_pbuf;	/* physical buffer address	*/
	unsigned	io_cnt;		/* transfer count		*/
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

	char            *ib_buf;

} ibarg_t;

#ifdef __KERNEL__
/* gpib_board_t is filled out by driver.  It is the interface
 * between the board-specific details dealt with in the drivers
 * and generic interface provided by gpib-common.
 *
 */
typedef struct
{
	char *name;	// name of board
	/* read() should read at most 'length' bytes from the bus into
	 * 'buffer'.  It should not return until it fills buffer or
	 * encounters an EOI (and or EOS if appropriate).  If 'eos'
	 * is nonzero, it is the end of string character the read should
	 * look for.  if 'eos' is zero, there is no end of string character.
	 * Ultimately, this will be changed into or replaced by an asynchronous
	 * read.  Positive return value is number of bytes read, negative
	 * return indicates error.
	 */
	ssize_t (*read)(uint8_t *buffer, size_t length, uint8_t eos);
	/* write() should write 'length' bytes from buffer to the bus.
	 * If the boolean value send_eoi is nonzero, then EOI should
	 * be sent along with the last byte.  Returns number of bytes
	 * written or negative value on error.
	 */
	ssize_t (*write)(uint8_t *buffer, size_t length, int send_eoi);
	/* Sends the command bytes in 'buffer' to the bus. 
	 */
	ssize_t (*command)(uint8_t *buffer, size_t length);
	/* Take control (assert ATN).  If 'asyncronous' is nonzero, take
	 * control asyncronously (assert ATN immediately without waiting
	 * for other processes to complete first).
	 */
	void (*take_control)(int asyncronous);
	/* De-assert ATN. */
	void (*go_to_standby)(void);
	/* Asserts or de-asserts 'interface clear' (IFC) depending on
	 * boolean value of 'assert'
	 */
	void (*interface_clear)(int assert);
	/* Sends remote enable command if 'enable' is nonzero, disables remote mode
	 * if 'enable' is zero
	 */
	void (*remote_enable)(int enable);
	/* enable END for reads, when byte 'eos' is received.  If
	 * 'compare_8_bits' is nonzero, then all 8 bits are compared
	 * with the eos bytes.  Otherwise only the 7 least significant
	 * bits are compared. */
	void (*enable_eos)(uint8_t eos, int compare_8_bits);
	/* disable END on eos byte (END on EOI only)*/
	void (*disable_eos)(void);
	/* suspend until one of the conditions specified by 'status_mask' is
	 * true.  The meaning of the bits in the 'status_mask' is the same
	 * as the meaning of the bits in the 'status' variable below.
	 * returns board's status.
	 */
	unsigned int (*wait)(unsigned int status_mask);
	/* TODO: conduct serial poll */
	void (*serial_poll)(void);
	/* TODO: conduct parallel poll */
	void (*parallel_poll)(void);
	/* Returns current status of the bus lines.  Should be set to
	 * NULL if your board does not have the ability to query the
	 * state of the bus lines. */
	int (*line_status)(void);
	/* Stores information on the board's current status.  Usually
	 * updated by the interrupt handler.  The meaning of the bits
	 * is specified in gpib_user.h in the IBSTA section. */
	int status;
	/* Holds error code for last error. */
	int error;
	/* 'private_data' can be used as seen fit by the driver to
	 * store additional variables for this board */
	void *private_data;
} gpib_board_t;

#endif	// __KERNEL__

#endif	// _GPIB_TYPES_H
