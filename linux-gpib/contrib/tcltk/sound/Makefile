######################################################################
#
#   Makefile for the sound tcl extension
#   (C) C.Schroeter clausi@chemie.fu-berlin.de
#
#####################################################################
#  
MAKE = make -C
#

include make.config

DEFINES= 

SUBDIRS = 

SRCS = soundCmds.c soundIt.o

OBJS = soundCmds.o soundIt.o 


LIBS =   
INCLUDES = -I.

CFLAGS = -fPIC $(INCLUDES) $(DEFINES)




all: libsound.so 
	
libsound.so:	$(OBJS)
	$(CC) -shared $(CFLAGS) $(OBJS) -o $@

clean: 
	-rm *.o *.a *.so make.config
	set -e;for i in $(SUBDIRS); do make -C $$i clean; done

install: all
	install libsound.so $(INSTALLDIR)

make.config:
	../config.tcl >make.config




