
#ifndef _GPIB_P_H
#define _GPIB_P_H

#include <linux/fs.h>

#include <autoconf.h>

#include <gpib_config.h>
#include <gpib_debug.h>
#include <gpib_types.h>

#include <gpib_proto.h>

#include <gpib_registers.h>
#include <gpib_ioctl.h>
#include <gpib_user.h>

void gpib_register_driver(gpib_interface_t *interface);
void gpib_unregister_driver(gpib_interface_t *interface);

#define MAX_NUM_GPIB_DEVICES 16
extern gpib_device_t device_array[MAX_NUM_GPIB_DEVICES];

extern struct list_head registered_drivers;

#endif	// _GPIB_P_H

