/***************************************************************************
                              ni_usb_gpib.h
                             -------------------

    begin                : Oct 2004
    copyright            : (C) 2004 by Frank Mori Hess
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

#ifndef _AGILENT_82357_H
#define _AGILENT_82357_H

#include <linux/usb.h>
#include "gpibP.h"

enum usb_vendor_ids
{
	USB_VENDOR_ID_AGILENT = 0x0957
};

enum usb_device_ids
{
	USB_DEVICE_ID_AGILENT_82357A = 0x0107,
	USB_DEVICE_ID_AGILENT__82357A_PREINIT = 0x0007	// device id before firmware is loaded
};

enum endpoint_addresses
{
	AGILENT_82357A_CONTROL_ENDPOINT = 0x0,
	AGILENT_82357A_BULK_IN_ENDPOINT = 0x2,
	AGILENT_82357A_BULK_OUT_ENDPOINT = 0x4,
	AGILENT_82357A_INTERRUPT_IN_ENDPOINT = 0x6,
};

enum bulk_commands
{
	DATA_PIPE_CMD_WRITE = 0x1,
	DATA_PIPE_CMD_READ = 0x3,
	DATA_PIPE_CMD_WR_REGS = 0x4,
	DATA_PIPE_CMD_RD_REGS = 0x5
};

enum agilent_82357a_read_flags
{
	ARF_END_ON_EOI = 0x1,
	ARF_NO_ADDRESS = 0x2,
	ARF_END_ON_EOS_CHAR = 0x4,
	ARF_SPOLL = 0x8
};

enum agilent_82357a_write_flags
{
	AWF_SEND_EOI = 0x1,
	AWF_NO_FAST_TALKER_FIRST_BYTE = 0x2,
	AWF_NO_FAST_TALKER = 0x4,
	AWF_NO_ADDRESS = 0x8,
	AWF_ATN = 0x10,
	AWF_SEPARATE_HEADER = 0x80
};
                        
enum agilent_82357_error_codes
{
	UGP_SUCCESS = 0,
	UGP_ERR_INVALID_CMD = 1,
	UGP_ERR_INVALID_PARAM = 2,
	UGP_ERR_INVALID_REG = 3,
	UGP_ERR_GPIB_READ = 4,
	UGP_ERR_GPIB_WRITE = 5,
	UGP_ERR_FLUSHING = 6,
	UGP_ERR_FLUSHING_ALREADY = 7,
	UGP_ERR_UNSUPPORTED = 8,
	UGP_ERR_OTHER  = 9
};

// struct which defines local data for each 82357 device
typedef struct
{
	struct usb_interface *bus_interface;
	unsigned short eos_char;
	unsigned short eos_mode;
	unsigned short hw_control_bits;
	struct semaphore bulk_transfer_lock;
	struct semaphore interrupt_transfer_lock;
	struct semaphore control_transfer_lock;
} agilent_82357a_private_t;

struct agilent_82357a_register_pairlet
{
	short address;
	unsigned short value;
};

enum firmware_registers
{
	HW_CONTROL = 0xa,
};

enum hardware_control_bits
{
	NOT_TI_RESET = 0x1,
	SYSTEM_CONTROLLER = 0x2,
	NOT_PARALLEL_POLL = 0x4,
	OSCILLATOR_5V_ON = 0x8,
	OUTPUT_5V_ON = 0x20,
	CPLD_3V_ON = 0x80,
};

#endif	// _AGILENT_82357_H
