#include <gpibP.h>

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/major.h>
#define __NO_VERSION__
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/timer.h>

#include <asm/io.h>
#include <asm/segment.h>
#include <asm/irq.h>
#include <asm/dma.h>
#include <asm/uaccess.h>
#include <asm/system.h>

