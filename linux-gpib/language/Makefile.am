# language/Makefile.am
# copyright (C) 2003 by Frank Mori Hess
# email : fmhess@users.sourceforge.net
#
#   This Makefile.am is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

SUBDIRS = $(PERL_SUBDIR) $(PYTHON_SUBDIR) $(PHP_SUBDIR) $(GUILE_SUBDIR) $(TCL_SUBDIR)

if BIND_PERL
PERL_SUBDIR=perl
else
PERL_SUBDIR=
endif

if BIND_PYTHON
PYTHON_SUBDIR=python
else
PYTHON_SUBDIR=
endif

if BIND_PHP
PHP_SUBDIR=php
else
PHP_SUBDIR=
endif

if BIND_GUILE
GUILE_SUBDIR=guile
else
GUILE_SUBDIR=
endif

if BIND_TCL
TCL_SUBDIR=tcl
else
TCL_SUBDIR=
endif
