# language/tcl/Makefile.am
# copyright (C) 2003 by Frank Mori Hess
# email : fmhess@users.sourceforge.net
#
#   This Makefile.am is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

lib_LTLIBRARIES = libgpib_tcl.la

libgpib_tcl_la_SOURCES = ibCmds.c

libgpib_tcl_la_CFLAGS = $(LIBGPIB_CFLAGS) -I/usr/include/tcl8.3
libgpib_tcl_la_LDFLAGS = -release $(VERSION) $(LIBGPIB_LDFLAGS)

