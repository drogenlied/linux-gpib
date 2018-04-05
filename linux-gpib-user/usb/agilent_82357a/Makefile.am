# Makefile.am
# copyright (C) 2004 by Frank Mori Hess
# email : fmhess@users.sourceforge.net
#
#   This Makefile.am is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

EXTRA_DIST = agilent_82357a agilent_82357a.usermap 99-agilent_82357a.rules.in
CLEANFILES = 99-agilent_82357a.rules
HOTPLUG_USB_CONF_DIR=/etc/hotplug/usb
USB_FIRMWARE_DIR=/usr/share/usb
UDEV_RULES_DIR=/etc/udev/rules.d

do_subst = sed -e 's,[@]sbindir[@],$(sbindir),g'  -e 's,[@]hotplug_usb_conf_dir[@],$(HOTPLUG_USB_CONF_DIR),g' 

99-agilent_82357a.rules: 99-agilent_82357a.rules.in
	$(do_subst) < 99-agilent_82357a.rules.in >99-agilent_82357a.rules

install-data-local: 99-agilent_82357a.rules
	test -e $(HOTPLUG_USB_CONF_DIR)/agilent_82357a || $(INSTALL_SCRIPT) -D agilent_82357a $(HOTPLUG_USB_CONF_DIR)/agilent_82357a
	test -e $(HOTPLUG_USB_CONF_DIR)/agilent_82357a.usermap || $(INSTALL_DATA) -D agilent_82357a.usermap $(HOTPLUG_USB_CONF_DIR)/agilent_82357a.usermap
	$(INSTALL) -d $(USB_FIRMWARE_DIR)/agilent_82357a
	test -d $(UDEV_RULES_DIR) && test -e $(UDEV_RULES_DIR)/99-agilent_82357a.rules || $(INSTALL_DATA) -D 99-agilent_82357a.rules $(UDEV_RULES_DIR)/99-agilent_82357a.rules
