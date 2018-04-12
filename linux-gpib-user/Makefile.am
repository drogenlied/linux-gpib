# Makefile.am
# copyright (C) 2003 by Frank Mori Hess
# email : fmhess@users.sourceforge.net
#
#   This Makefile.am is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

SUBDIRS = include lib drivers examples test doc language usb

EXTRA_DIST = etc util m4 README.HAMEG README.hp82335 bootstrap

ACLOCAL_AMFLAGS = -I m4

distclean-local:
	$(RM) $(srcdir)/util/.scm_version.tmp

dist-hook:
	-find $(distdir) -depth -name '.svn' -type d -exec $(RM) -r \{\} \;
	-find $(distdir) \( -name '*~' -o -name '.#*' \) -exec $(RM) \{\} \;
	$(RM) $(distdir)/util/.scm_version.tmp

.PHONY : device_files
device_files: 
	cd drivers && $(MAKE) $(AM_MAKEFLAGS) device_files
