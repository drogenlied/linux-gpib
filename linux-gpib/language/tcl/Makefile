######################################################################
#
#   Makefile for the gpib tcl extension
#   (C) C.Schroeter clausi@chemie.fu-berlin.de
#
#####################################################################

#####################################################################
# 
# The Path where your GPIB stuff lives
# This is normally set by the toplevel Makefile
#
GPIB_HOME = `(cd ../.. ; pwd)`

include make.config

#######################################################################
# The switches for your GPIB library
#
#
GPIBLIBS 	= -L$(GPIB_HOME)/lib -lgpib
GPIBINCLUDE 	= -I$(GPIB_HOME)/include

######################################################################
################### End Editable Parameters ##########################

MAKE = make -C
#
#

SUBDIRS = $(EXTSUBDIRS) 


LIBS =  $(GPIBLIBS) -lfl -lm
INCLUDES = -I. -I$(GPIB_HOME)/include 

CFLAGS = -fpic $(INCLUDES) $(DEFINES)


SRCS = ibCmds.c 

OBJS = ibCmds.o 


all: libgpib.so

libgpib.so: $(OBJS)
	$(CC) -shared $(OBJS) $(LIBS) -o $@

clean: 
	-rm *.o *.so make.config
	set -e;for i in $(SUBDIRS); do make -C $$i clean; done

realclean: 
	-rm *.o ibsh
	set -e;for i in $(SUBDIRS) ./modules; do make -C $$i clean; done

install:
	install ./libgpib.so $(INSTALLDIR)	

make.config:
	tclsh config.tcl >make.config





