# pc2/Makefile.am
# copyright (C) 2003 by Frank Mori Hess
# email : fmhess@users.sourceforge.net
#
#   This Makefile.am is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

EXTRA_DIST = $(SRCS) pc2.h

-include $(top_srcdir)/util/linux_flags/flags
CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/driver/include -I$(top_srcdir)/include
CFLAGS = $(CFLAGS_LINUX)
CC = $(CC_LINUX)
LD = $(LD_LINUX)
MODLIB = $(MODLIB_LINUX)

SUFFIXES = .c .o

SRCS = pc2_init.c interrupt.c

OBJS = $(SRCS:.c=.o)

MODULE = pc2_gpib.o

all-local: $(MODULE)

clean-local:
	$(RM) *.o

$(MODULE): $(OBJS)
	$(LD) -r -o $(MODULE) $(OBJS)

install-data-local:
	$(INSTALL_DATA) -D $(MODULE) $(MODLIB)/gpib/$(MODULE)

#automatic dependency handling
-include $(SRCS:.c=.d)
%.d: %.c
	set -e; $(CC) -MM $(CPPFLAGS) $< \
	| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
	[ -s $@ ] || rm -f $@



