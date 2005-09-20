/*
 * linux/device.h compatibility header
 */

#ifndef __COMPAT_LINUX_DEVICE_H_
#define __COMPAT_LINUX_DEVICE_H_

#include <linux/version.h>

#include_next <linux/device.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13)
#define class_create(owner, name) \
	(struct class *)class_simple_create(owner, name)
#define class_destroy(cs) \
	class_simple_destroy((struct class_simple *)(cs))
#define class_device_create(cs, dev, device, fmt...) \
	class_simple_device_add((struct class_simple *)(cs), \
		dev, device, fmt)
#define class_device_destroy(cs, dev) \
	class_simple_device_remove(dev)
#endif

#endif // __COMPAT_LINUX_DEVICE_H_

