/***************************************************************************
                          gpib_ioctl.h  -  header file for gpib library
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


#ifndef _GPIB_IOCTL_H
#define _GPIB_IOCTL_H

#include <asm/ioctl.h>

#define GPIB_CODE 160

typedef struct
{
	char name[100];
} board_type_ioctl_t;

// argument for read/write/command ioctls
typedef struct
{
	uint8_t *buffer;
	unsigned long count;
	int end;
} read_write_ioctl_t;

typedef struct
{
	unsigned int pad;
	int sad;
} open_close_dev_ioctl_t;

typedef struct
{
	unsigned int pad;
	int sad;
	uint8_t status_byte;
} serial_poll_ioctl_t;

typedef struct
{
	int eos;
	int eos_flags;
} eos_ioctl_t;

typedef struct
{
	int mask;
	unsigned int pad;
	int sad;
	unsigned long usec_timeout;
} wait_ioctl_t;

typedef struct
{
	int online;
	int master;
} online_ioctl_t;

typedef struct
{
	unsigned int num_bytes;
	unsigned int pad;
	int sad;
} spoll_bytes_ioctl_t;

typedef struct
{
	unsigned int pad;
	int sad;
	int parallel_poll_configuration;
	int autopolling;
} board_info_ioctl_t;

/* Standard functions. */
#define IBRD _IOWR( GPIB_CODE, 0, read_write_ioctl_t )
#define IBWRT _IOWR( GPIB_CODE, 1, read_write_ioctl_t )
#define IBCMD _IOWR( GPIB_CODE, 2, read_write_ioctl_t )
#define IBOPENDEV _IOW( GPIB_CODE, 3, open_close_dev_ioctl_t )
#define IBCLOSEDEV _IOW( GPIB_CODE, 4, open_close_dev_ioctl_t )
#define IBWAIT _IOW( GPIB_CODE, 5, wait_ioctl_t )
#define IBRPP _IOWR( GPIB_CODE, 6, uint8_t )

#define IBONL _IOW( GPIB_CODE, 8, online_ioctl_t )
#define IBSIC _IO( GPIB_CODE, 9 )
#define IBSRE _IOW( GPIB_CODE, 10, int )
#define IBGTS _IO( GPIB_CODE, 11 )
#define IBCAC _IOW( GPIB_CODE, 12, int )
#define IBSTATUS _IOR( GPIB_CODE, 13, int )
#define IBLINES _IOR( GPIB_CODE, 14, short )
#define IBPAD _IOW( GPIB_CODE, 15, unsigned int )
#define IBSAD _IOW( GPIB_CODE, 16, int )
#define IBTMO _IOW( GPIB_CODE, 17, unsigned int )
#define IBRSP		_IOWR( GPIB_CODE, 18, serial_poll_ioctl_t )
#define IBEOS _IOW( GPIB_CODE, 19, eos_ioctl_t )
#define IBRSV _IOW( GPIB_CODE, 20, uint8_t )
#define CFCBASE _IOW( GPIB_CODE, 21, unsigned long )
#define CFCIRQ _IOW( GPIB_CODE, 22, unsigned int )
#define CFCDMA _IOW( GPIB_CODE, 23, unsigned int )
#define CFCBOARDTYPE _IOW( GPIB_CODE, 24, board_type_ioctl_t )
#define IBAUTOPOLL _IO( GPIB_CODE, 25 )
#define IBMUTEX _IOW( GPIB_CODE, 26, int )
#define IBSPOLL_BYTES _IOWR( GPIB_CODE, 27, spoll_bytes_ioctl_t )
#define IBPPC _IOW( GPIB_CODE, 28, int )
#define IBBOARD_INFO _IOR( GPIB_CODE, 29, board_info_ioctl_t )

#define IBQUERY_BOARD_RSV _IOR( GPIB_CODE, 31, int )

#endif	/* _GPIB_IOCTL_H */
