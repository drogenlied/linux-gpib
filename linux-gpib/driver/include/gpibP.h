
#ifndef _GPIB_P_H
#define _GPIB_P_H

#include <linux/fs.h>

#include <autoconf.h>

#include <gpib_types.h>

#include <gpib_proto.h>

#include <gpib_registers.h>
#include <gpib_ioctl.h>
#include <gpib_user.h>

void gpib_register_driver(gpib_interface_t *interface);
void gpib_unregister_driver(gpib_interface_t *interface);
int gpib_clear_to_write( gpib_board_t *board );

#define MAX_NUM_GPIB_BOARDS 16
extern gpib_board_t board_array[MAX_NUM_GPIB_BOARDS];

extern struct list_head registered_drivers;

#if defined( GPIB_CONFIG_VERBOSE_DEBUG )
#define GPIB_DPRINTK( format, args... ) printk( "gpib debug: " format, ## args )
#else
#define GPIB_DPRINTK( arg... )
#endif

#include <asm/io.h>

void writeb_wrapper( unsigned int value, unsigned long address );
unsigned int readb_wrapper( unsigned long address );
void outb_wrapper( unsigned int value, unsigned long address );
unsigned int inb_wrapper( unsigned long address );

#endif	// _GPIB_P_H

