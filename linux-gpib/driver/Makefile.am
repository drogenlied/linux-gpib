# driver/Makefile.am
# copyright (C) 2003 by Frank Mori Hess
# email : fmhess@users.sourceforge.net
#
#   This Makefile.am is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

SUBDIRS = cb7210 cec hp82335 ines nec7210 pc2 sys tnt4882 tnt4882-tms include

#should use this if I convert driver makefiles to automake
#AUTOMAKE_OPTIONS=nostdinc

-include $(top_srcdir)/util/linux_flags/flags
CFLAGS = -I$(shell cd $(top_srcdir);pwd)/ -I$(shell cd $(top_srcdir);pwd)/driver/include -I$(shell cd $(top_srcdir);pwd)/include $(CFLAGS_LINUX)
CC = $(CC_LINUX)
LD = $(LD_LINUX)
MODLIB = $(MODLIB_LINUX)
export CC CFLAGS LD MODLIB INSTALL


