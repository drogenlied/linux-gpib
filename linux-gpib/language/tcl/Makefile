######################################################################
#
#   Makefile for the gpib tcl extension
#   (C) C.Schroeter clausi@chemie.fu-berlin.de
#
#####################################################################

-include make.config

#######################################################################
# The switches for your GPIB library
#
#
GPIBLIBS 	= -L$(ROOT)/lib -lgpib
GPIBINCLUDE 	= -I$(ROOT)/include 

######################################################################
################### End Editable Parameters ##########################

MAKE = make -C
#
#

SUBDIRS = examples 


LIBS =  $(GPIBLIBS) -lfl -lm
INCLUDES = -I. -I$(ROOT)/include -I$(ROOT)/driver/include -I/usr/include/tcl8.3 

CFLAGS = -fpic $(INCLUDES) $(DEFINES) -Wall


SRCS = ibCmds.c 

OBJS = ibCmds.o 


all: libgpib-tcl.so

libgpib-tcl.so: $(OBJS)
	$(CC) -shared $(OBJS) $(LIBS) -o $@

clean: 
	rm -f *.o *.so make.config
	set -e;for i in $(SUBDIRS); do make -C $$i clean; done

realclean: 
	rm -f *.o ibsh
	set -e;for i in $(SUBDIRS) ./modules; do make -C $$i clean; done

install:
	install ./libgpib-tcl.so $(INSTALLDIR)	

make.config:
	tclsh config.tcl >make.config





