# tms9914/Makefile.am
# copyright (C) 2003 by Frank Mori Hess
# email : fmhess@users.sourceforge.net
#
#   This Makefile.am is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

EXTRA_DIST = $(SRCS) board.h

include $(top_srcdir)/util/linux_flags/flags
CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/driver/include -I$(top_srcdir)/include \
	$(CFLAGS_LINUX) -DEXPORT_SYMTAB
CC = $(CC_LINUX)
LD = $(LD_LINUX)
MODLIB = $(MODLIB_LINUX)

SUFFIXES = .c .o

SRCS = cmd.c init.c read.c util.c write.c aux.c interrupt.c

OBJS = $(SRCS:.c=.o)

MODULE = tms9914.o

all-local: $(MODULE)

clean-local:
	$(RM) *.o

$(MODULE): $(OBJS)
	$(LD) -r -o $(MODULE) $(OBJS)

install-data-local:
	$(INSTALL_DATA) -D $(MODULE) $(MODLIB)/gpib/$(MODULE)



