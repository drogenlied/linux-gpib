# sys/Makefile.am
# copyright (C) 2003 by Frank Mori Hess
# email : fmhess@users.sourceforge.net
#
#   This Makefile.am is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

MODULE_EXTRA_DIST = autopoll.h ibsys.h
MODULE_CFLAGS = -DEXPORT_SYMTAB
MODULE_SRCS = osfuncs.c  osinit.c  ostimer.c osutil.c autopoll.c ibcac.c ibcmd.c \
	ibgts.c ibinit.c iblines.c ibread.c ibrpp.c ibrsv.c ibsic.c \
	ibsre.c ibutil.c ibwait.c ibwrite.c device.c event.c
MODULE = gpib_common.o

include $(top_srcdir)/driver/Makefile-driver.am


