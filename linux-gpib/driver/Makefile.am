# driver/Makefile.am
# copyright (C) 2003 by Frank Mori Hess
# email : fmhess@users.sourceforge.net
#
#   This Makefile.am is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

GPIB_MODULE_SUBDIRS :=cb7210 cec hp82335 ines nec7210 pc2 sys tms9914 tnt4882
GPIB_ABS_MODULE_SUBDIRS :=$(patsubst %, @abs_srcdir@/%/, $(GPIB_MODULE_SUBDIRS))
GPIB_MODULES :=cb7210/cb7210.o cec/cec_gpib.o hp82335/hp82335.o \
	ines/ines_gpib.o pc2/pc2_gpib.o tms9914/tms9914.o tnt4882/tnt4882.o \
	nec7210/nec7210.o tms9914/tms9914.o sys/gpib_common.o

EXTRA_DIST = $(GPIB_MODULE_SUBDIRS) tnt4882-tms

SUBDIRS = include

-include $(top_srcdir)/util/linux_flags/flags

all-local:
	$(MAKE) -C $(LINUX_SRCDIR) modules V=1 \
		CC="$(CC_LINUX) -I@abs_top_srcdir@ -I@abs_top_srcdir@/driver/include -I@abs_top_srcdir@/include" \
		SUBDIRS="$(GPIB_ABS_MODULE_SUBDIRS)"

clean-local:
	find $(builddir) \( -name '*.[oas]' -o -name core -o -name '.*.flags' \) -type f \
		-exec $(RM) {} \;

install-data-hook: device-file-check /etc/gpib.conf
	for module in $(GPIB_MODULES) ; do \
		$(INSTALL_DATA) -D $$module $(MODLIB_LINUX)/gpib/$$(basename $$module) ; \
	done
	$(DEPMOD) -ae

/dev/gpib0:
	groupadd gpib || echo "group gpib exists"
	for i in $(shell seq 0 15); \
	do \
		mknod --mode u=rw,g=rw,o= /dev/gpib$${i} c $(IBMAJOR) $${i} || exit 1; \
		chown root.gpib /dev/gpib$${i}; \
	done

.PHONY : device-file-check
device-file-check: /dev/gpib0
	@if [ ! -c /dev/gpib0 ]; then \
		if [ -a /dev/gpib0 ]; then \
			echo "A file or directory called /dev/gpib0 exists but it is not" \
				"a character device.  Delete or move it and try again."; \
			exit 1; \
		fi; \
	fi
	@ls -l /dev/gpib0 | grep -q "$(IBMAJOR)"; \
	if [ $$? != 0 ]; then \
		echo "/dev/gpib0 has the wrong major number. " \
			"Delete your /dev/gpibX files and try again."; \
		exit 1; \
	fi

#should move this to util/templates Makefile.am when it exists
/etc/gpib.conf:
	$(INSTALL_DATA) -D $(top_srcdir)/util/templates/gpib.conf /etc/gpib.conf

