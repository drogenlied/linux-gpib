# Makefile.am
# copyright (C) 2004 by Frank Mori Hess
# email : fmhess@users.sourceforge.net
#
#   This Makefile.am is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

EXTRA_DIST = agilent_82357a.in 99-agilent_82357a.rules.in
CLEANFILES = 99-agilent_82357a.rules agilent_82357a
USB_FIRMWARE_DIR=$(datadir)/usb
UDEV_RULES_DIR=$(sysconfdir)/udev/rules.d
UDEV_LIB_DIR=$(libdir)/udev

do_subst = $(SED) -e 's,[@]sbindir[@],$(sbindir),g'\
	-e 's,[@]datadir[@],$(datadir),g'\
	-e 's,[@]libdir[@],$(libdir),g'

99-agilent_82357a.rules: 99-agilent_82357a.rules.in
	$(do_subst) < $(srcdir)/99-agilent_82357a.rules.in >99-agilent_82357a.rules

agilent_82357a: agilent_82357a.in
	$(do_subst) < $(srcdir)/agilent_82357a.in >agilent_82357a

install-data-local: 99-agilent_82357a.rules agilent_82357a
	test -e $(DESTDIR)$(UDEV_LIB_DIR)/agilent_82357a ||\
		$(INSTALL_SCRIPT) -D agilent_82357a $(DESTDIR)$(UDEV_LIB_DIR)/agilent_82357a
	$(INSTALL) -d $(DESTDIR)$(USB_FIRMWARE_DIR)/agilent_82357a
	test -d $(DESTDIR)$(UDEV_RULES_DIR) && test -e $(DESTDIR)$(UDEV_RULES_DIR)/99-agilent_82357a.rules ||\
		$(INSTALL_DATA) -D 99-agilent_82357a.rules $(DESTDIR)$(UDEV_RULES_DIR)/99-agilent_82357a.rules
