#
#  Makefile for Linux GPIB
#

ROOT = $(PWD)
MAKE = make -C

INCDIR  = include
SUBDIRS = driver lib util examples language applications

PKG_SRC = $(SUBDIRS) include Makefile
DOCDIRS = doc

export ROOT

all: linux-gpib .test.stat

linux-gpib: .config.stat 
	set -e; \
	for i in $(INCDIR) $(SUBDIRS); do \
		$(MAKE) $$i all; \
	done

doc: .config.stat 
	set -e; \
	for i in $(INCDIR) $(DOCDIRS); do \
		$(MAKE) $$i all; \
	done

clean:
	set -e; \
	for i in $(INCDIR) $(SUBDIRS); do \
		$(MAKE) $$i NODEPS=y $@; \
	done
	$(RM) *.o .config.stat .test.stat

rcsput rcsget:
	set -e; \
	for i in $(SUBDIRS); do \
		$(MAKE) $$i $@; \
	done


package:
	tar cvf /dev/fd1 $(PKG_SRC)


includes:
	-for i in $(SUBDIRS); do \
		ln -s $$i/*.h ./include; \
	done

load:
	set -e ;make -C driver load

unload:
	set -e ;make -C driver unload
	

install: all
	set -e; \
	for i in $(SUBDIRS) $(INCDIR); do \
		$(MAKE) $$i -k install ; \
	done
	(cd ./util; ./Setup.install)


.config.stat config: 
	(cd ./util; ./Setup)
	touch .config.stat

.test.stat test:
	(cd ./util; ./Setup --test)
	touch .test.stat



include $(ROOT)/makefile.inc




