#
#  Makefile for Linux GPIB
#

ROOT = $(PWD)
MAKE = make 

INCDIR  = include
SUBDIRS = driver lib examples applications

PKG_SRC = $(SUBDIRS) include Makefile
DOCDIRS = doc


all: linux-gpib 

linux-gpib: .config.stat 
	set -e; \
	for i in $(INCDIR) $(SUBDIRS); do \
		$(MAKE) -C $$i all; \
	done

doc: .config.stat 
	set -e; \
	for i in $(INCDIR) $(DOCDIRS); do \
		$(MAKE) -C $$i all; \
	done

clean:
	set -e; \
	for i in $(INCDIR) $(SUBDIRS); do \
		$(MAKE) -C $$i NODEPS=y $@; \
	done
	rm -f *.o 

rcsput rcsget:
	set -e; \
	for i in $(SUBDIRS); do \
		$(MAKE) -C $$i $@; \
	done


package:
	tar cvf /dev/fd1 $(PKG_SRC)


#includes:
#	-for i in $(SUBDIRS); do \
#		ln -s $$i/*.h ./include; \
#	done

load:
	set -e ;make -C driver load

unload:
	set -e ;make -C driver unload
	

install: all
	set -e; \
	for i in $(SUBDIRS) $(INCDIR); do \
		$(MAKE) -C $$i -k install ; \
	done
	(cd ./util; ./Setup.install)
	
.config.stat config: 
	(cd ./util; ./Setup)
	touch .config.stat


include $(ROOT)/makefile.inc

export 



