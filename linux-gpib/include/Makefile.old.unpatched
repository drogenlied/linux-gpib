#
#  Makefile for ieee488 driver
#

INSTALLDIR=/usr/local/include
MAKE = make -C

all: includes

includes:
	-for i in ../lib ../driver/include ; do ln -s $$i/*.h .; done


install:
	install -d $(INSTALLDIR)/gpib
	for i in *.h; do install $$i $(INSTALLDIR)/gpib;done
