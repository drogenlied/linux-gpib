# language/php/Makefile.am
# copyright (C) 2003 by Frank Mori Hess
# email : fmhess@users.sourceforge.net
#
#   This Makefile.am is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

lib_LTLIBRARIES = gpib_php.la

gpib_php_la_SOURCES = gpib.c

EXTENSIONDIR = $(shell php-config --extension-dir)
PHP_INCLUDES = $(shell php-config --includes)

gpib_php_la_CFLAGS = $(LIBGPIB_CFLAGS) -DUSE_LINUX_GPIB $(PHP_INCLUDES)
gpib_php_la_LDFLAGS = -release $(VERSION) $(LIBGPIB_LDFLAGS) -module
libdir = $(EXTENSIONDIR)

