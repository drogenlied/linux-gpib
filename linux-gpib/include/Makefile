#
#  Makefile for ieee488 driver
#

ROOT = ..
INSTALLDIR=/usr/local/include
MAKE = make -C

all:	gpib

gpib:
	$(ECHO) --- Creating softlink $(BOLD)gpib$(NBOLD)...
	ln -s . gpib



install:
	if [ ! -d $(INSTALLDIR)/gpib ]; then \
		mkdir $(INSTALLDIR)/gpib ;\
	fi
	for i in *.h; do \
		$(INSTALL) $$i $(INSTALLDIR)/gpib/ ;\
	done

	
clean:
	rm -f gpib


.SILENT:

include $(ROOT)/makefile.inc

