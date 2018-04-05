# include/Makefile.am
# copyright (C) 2003 by Frank Mori Hess
# email : fmhess@users.sourceforge.net
#
#   This Makefile.am is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

BUILT_SOURCES = gpib ib.h ibConf.h ibP.h

headersdir = $(includedir)/gpib
headers_HEADERS = gpib_user.h
noinst_HEADERS = gpib_ioctl.h

gpib:
	ln -sf . gpib

ib.h:
	ln -sf $(top_srcdir)/lib/ib.h

ibConf.h:
	ln -sf $(top_srcdir)/lib/ibConf.h

ibP.h:
	ln -sf $(top_srcdir)/lib/ibP.h

