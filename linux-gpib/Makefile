#
#  Makefile for Linux GPIB
#

.SILENT:

ROOT = .
MAKE = make -C

INCDIR  = include
SUBDIRS = driver lib util examples language applications

PKG_SRC = $(SUBDIRS) include Makefile
DOCDIRS = doc

all: linux-gpib .test.stat

linux-gpib: .config.stat 
	set -e; \
	for i in $(INCDIR) $(SUBDIRS); do \
		$(ECHO) "|||" Making target $(BOLD)all$(NBOLD) in subdirectory $(BOLD)$$i$(NBOLD)... ; \
		$(MAKE) $$i all; \
		$(ECHO) "|||" Leaving subdirectory $(BOLD)$$i$(NBOLD)...; \
	done

doc: .config.stat 
	set -e; \
	for i in $(INCDIR) $(DOCDIRS); do \
		$(ECHO) "|||" Making target $(BOLD)all$(NBOLD) in subdirectory $(BOLD)$$i$(NBOLD)... ; \
		$(MAKE) $$i all; \
		$(ECHO) "|||" Leaving subdirectory $(BOLD)$$i$(NBOLD)...; \
	done

clean:
	set -e; \
	for i in $(INCDIR) $(SUBDIRS); do \
		$(ECHO) "|||" Making target $(BOLD)$@$(NBOLD) in subdirectory $(BOLD)$$i$(NBOLD)... ; \
		$(MAKE) $$i NODEPS=y $@; \
		$(ECHO) "|||" Leaving subdirectory $(BOLD)$$i$(NBOLD)...; \
	done
	$(RM) *.o .config.stat .test.stat

rcsput rcsget:
	set -e; \
	for i in $(SUBDIRS); do \
		$(ECHO) "|||" Making target $(BOLD)$@$(NBOLD) in subdirectory $(BOLD)$$i$(NBOLD)... ; \
		$(MAKE) $$i $@; \
		$(ECHO) "|||" Leaving subdirectory $(BOLD)$$i$(NBOLD)...; \
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
		$(ECHO) "|||" Making target $(BOLD)$@$(NBOLD) in subdirectory $(BOLD)$$i$(NBOLD)... ; \
		$(MAKE) $$i -k install ; \
		$(ECHO) "|||" Leaving subdirectory $(BOLD)$$i$(NBOLD)...; \
	done
	(cd ./util; ./Setup.install)


.config.stat config: 
	(cd ./util; ./Setup)
	touch .config.stat

.test.stat test:
	(cd ./util; ./Setup --test)
	touch .test.stat



include $(ROOT)/makefile.inc




