# Makefile.am
# copyright (C) 2003 by Frank Mori Hess
# email : fmhess@users.sourceforge.net
#
#   This Makefile.am is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

SUBDIRS = include lib examples test driver doc language usb

EXTRA_DIST = etc util m4 README.HAMEG README.hp82335 bootstrap

ACLOCAL_AMFLAGS = -I m4

dist-hook:
	-find $(distdir) -name CVS -exec $(RM) -r {} \;
	-find $(distdir) \( -name '*~' -o -name '.#*' -o -name '*.mod.c' -o -name '.depend' \) -exec $(RM) {} \;
	$(RM) $(distdir)/util/linux_flags/fake.* 
	$(RM) $(distdir)/util/linux_flags/.*.cmd

