/*
 * pcmcia/ds.h compatibility header
 */
/*
    Copyright (C) 2010 Frank Mori Hess <fmhess@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __COMPAT_PCMCIA_DS_H_
#define __COMPAT_PCMCIA_DS_H_

#include <linux/version.h>
#include_next <pcmcia/ds.h>
#include <linux/device.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
struct pcmcia_driver {
	dev_link_t *(*attach) (void);
	void (*detach) (dev_link_t *);
	struct module *owner;
	struct device_driver drv;
};

/* driver registration */
static inline int pcmcia_register_driver(struct pcmcia_driver *driver)
{
	return register_pccard_driver((dev_info_t *) driver->drv.name,
		driver->attach, driver->detach);
};

static void inline pcmcia_unregister_driver(struct pcmcia_driver *driver)
{
	unregister_pccard_driver((dev_info_t *) driver->drv.name);
};

static void inline cs_error(client_handle_t handle, int func, int ret)
{
	error_info_t err = { func, ret };
	CardServices(ReportError, handle, &err);
};

#endif

static inline int compat_pcmcia_map_mem_page
(
	struct pcmcia_device *p_dev,
	window_handle_t win,
    memreq_t *req
)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
	return pcmcia_map_mem_page( win, req );
#else
	return pcmcia_map_mem_page( p_dev, win, req );
#endif
};

static inline int compat_pcmcia_request_window
(
	struct pcmcia_device *p_dev,
	win_req_t *req,
	window_handle_t *wh
)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
	return pcmcia_request_window(&p_dev, req, wh);
#else
	return pcmcia_request_window(p_dev, req, wh);
#endif
}

static inline int compat_pcmcia_release_window
(
	struct pcmcia_device *p_dev,
	window_handle_t win
)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
	return pcmcia_release_window(win);
#else
	return pcmcia_release_window(p_dev, win);
#endif
}

static inline unsigned compat_pcmcia_get_irq_line( struct pcmcia_device *p_dev)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
	return p_dev->irq.AssignedIRQ;
#else
	return p_dev->irq;
#endif
}

#endif // __COMPAT_PCMCIA_DS_H_
