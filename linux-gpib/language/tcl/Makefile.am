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

TCL_LIBRARY = $(shell echo 'puts $$tcl_library' | tclsh)
TCL_CONFIG = $(TCL_LIBRARY)/tclConfig.sh

TCL_INC_DIR = $(shell echo 'echo $$TCL_INC_DIR' | cat $(TCL_CONFIG) - | $(SHELL))
TCL_LIB_SPEC = $(shell echo 'echo $$TCL_LIB_SPEC' | cat $(TCL_CONFIG) - | $(SHELL))

#fix tcl directory for version independence
libgpib_tcl_la_CFLAGS = $(LIBGPIB_CFLAGS) -I$(TCL_INC_DIR)
libgpib_tcl_la_LDFLAGS = -release $(VERSION) $(LIBGPIB_LDFLAGS) $(TCL_LIB_SPEC)

