/***************************************************************************
                              ni_usb_gpib.h
                             -------------------

    begin                : May 2004
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

#ifndef _NI_USB_GPIB_H
#define _NI_USB_GPIB_H

#include <linux/usb.h>
#include "gpibP.h"

enum 
{
	USB_VENDOR_ID_NI = 0x3923
};

enum
{
	USB_DEVICE_ID_NI_USB_B = 0x702a,
	USB_DEVICE_ID_NI_USB_B_PREINIT = 0x702b	// device id before firmware is loaded
};

enum ni_usb_device
{
	NIUSB_SUBDEV_TNT4882 = 1,
	NIUSB_SUBDEV_UNKNOWN2 = 2,
	NIUSB_SUBDEV_UNKNOWN3 = 3,
};

enum endpoint_addresses
{
	NIUSB_BULK_OUT_ENDPOINT = 0x2,
	NIUSB_BULK_IN_ENDPOINT = 0x2,
	NIUSB_INTERRUPT_IN_ENDPOINT = 0x4,
};

// struct which defines private_data for ni_usb devices
typedef struct
{
	struct usb_interface *bus_interface;
	uint8_t eos_char;
	unsigned short eos_mode;
	unsigned int monitored_ibsta_bits;
	struct urb *interrupt_urb;
	uint8_t interrupt_buffer[0x11];
	struct semaphore bulk_transfer_lock;
	struct semaphore control_transfer_lock;
	struct semaphore interrupt_transfer_lock;
} ni_usb_private_t;

struct ni_usb_status_block
{
	short id;
	unsigned short ibsta;
	short error_code;
	unsigned short count;
};

struct ni_usb_register
{
	enum ni_usb_device device;
	short address;
	unsigned short value;
};

enum ni_usb_bulk_ids
{
	NIUSB_IBCAC_ID = 0x1,
	NIUSB_UNKNOWN3_ID = 0x3, // device level function id?
	NIUSB_TERM_ID = 0x4,
	NIUSB_IBGTS_ID = 0x6,
	NIUSB_IBRPP_ID = 0x7,
	NIUSB_REG_READ_ID = 0x8,
	NIUSB_REG_WRITE_ID = 0x9,
	NIUSB_IBSIC_ID = 0xf,
	NIUSB_REGISTER_READ_DATA_START_ID = 0x34,
	NIUSB_REGISTER_READ_DATA_END_ID = 0x35,
	NIUSB_IBRD_DATA_ID = 0x36,
	NIUSB_IBRD_STATUS_ID = 0x38
};

static const int ni_usb_control_request = 0x21;

static const unsigned int ni_usb_ibsta_monitor_mask = SRQI | LOK | REM | CIC | ATN | TACS | LACS | DTAS | DCAS;

static inline int nec7210_to_tnt4882_offset(int offset)
{
	return 2 * offset;
};
static inline int ni_usb_bulk_termination(uint8_t *buffer)
{
	int i = 0;
	
	buffer[i++] = NIUSB_TERM_ID;
	buffer[i++] = 0x0;
	buffer[i++] = 0x0;
	buffer[i++] = 0x0;
	return i;	
}

static inline int ni_usb_bulk_register_write_header(uint8_t *buffer, int num_writes)
{
	int i = 0;
	
	buffer[i++] = NIUSB_REG_WRITE_ID;
	buffer[i++] = num_writes;
	buffer[i++] = 0x0;
	return i;	
}

static inline int ni_usb_bulk_register_write(uint8_t *buffer, struct ni_usb_register reg)
{
	int i = 0;
	
	buffer[i++] = reg.device;
	buffer[i++] = reg.address;
	buffer[i++] = reg.value;
	return i;	
}

static inline int ni_usb_bulk_register_read_header(uint8_t *buffer, int num_reads)
{
	int i = 0;
	
	buffer[i++] = NIUSB_REG_READ_ID;
	buffer[i++] = num_reads;
	return i;	
}

static inline int ni_usb_bulk_register_read(uint8_t *buffer, int device, int address)
{
	int i = 0;
	
	buffer[i++] = device;
	buffer[i++] = address;
	return i;	
}

#endif	// _NI_USB_GPIB_H
