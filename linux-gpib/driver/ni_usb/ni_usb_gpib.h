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

static const int USB_VENDOR_ID_NI = 0x3923;
static const int USB_DEVICE_ID_NI_USB_B = 0x702a;

enum ni_usb_devices
{
	NI_USB_SUBDEV_TNT4882 = 1,
	NI_USB_SUBDEV_UNKNOWN = 2,
};

// struct which defines private_data for ni_usb devices
typedef struct
{
	struct usb_interface *bus_interface;
} ni_usb_private_t;

struct ni_usb_status_block
{
	short id;
	unsigned short ibsta;
	short error_code;
	unsigned short count;
};

static inline nec7210_to_tnt4882_offset(int offset)
{
	return 2 * offset;
};
static inline int ni_usb_bulk_termination(uint8_t *buffer)
{
	int i = 0;
	
	buffer[i++] = 0x4;
	buffer[i++] = 0x0;
	buffer[i++] = 0x0;
	buffer[i++] = 0x0;
	return i;	
}

static inline int ni_usb_bulk_register_write_header(uint8_t *buffer, int num_writes)
{
	int i = 0;
	
	buffer[i++] = 0x9;
	buffer[i++] = num_writes;
	buffer[i++] = 0x0;
	return i;	
}

static inline int ni_usb_bulk_register_write(uint8_t *buffer, int device, int address, int value)
{
	int i = 0;
	
	buffer[i++] = device;
	buffer[i++] = address;
	buffer[i++] = value;
	return i;	
}

#endif	// _NI_USB_GPIB_H
