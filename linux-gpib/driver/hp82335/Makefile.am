# hp82335/Makefile.am
# copyright (C) 2003 by Frank Mori Hess
# email : fmhess@users.sourceforge.net
#
#   This Makefile.am is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

include $(top_srcdir)/util/linux_flags/flags
CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/driver/include -I$(top_srcdir)/include \
	$(CFLAGS_LINUX)
CC = $(CC_LINUX)
LD = $(LD_LINUX)
MODLIB = $(MODLIB_LINUX)

SUFFIXES = .c .o

SRCS = hp82335_init.c

OBJS = $(SRCS:.c=.o)

MODULE = hp82335.o

all-local: $(MODULE)

clean-local:
	$(RM) *.o

$(MODULE): $(OBJS)
	$(LD) -r -o $(MODULE) $(OBJS)

install-data-local:
	$(INSTALL_DATA) -D $(MODULE) $(MODLIB)/misc/$(MODULE)



