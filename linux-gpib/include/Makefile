#
#

ROOT = ..
INSTALLDIR=/usr/local/include
MAKE = make -C

all:	gpib

gpib:
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


include $(ROOT)/makefile.inc

