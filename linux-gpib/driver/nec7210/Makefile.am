# nec7210/Makefile.am
# copyright (C) 2003 by Frank Mori Hess
# email : fmhess@users.sourceforge.net
#
#   This Makefile.am is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

MODULE_EXTRA_DIST = board.h
MODULE_CFLAGS = -DEXPORT_SYMTAB
MODULE_SRCS = cmd.c init.c read.c util.c write.c aux.c interrupt.c
MODULE = nec7210.o

include $(top_srcdir)/driver/Makefile-driver.am

