######################################################################
#
#   Makefile for the meters tcl extensions
#   (C) C.Schroeter clausi@chemie.fu-berlin.de
#
#####################################################################
#  
MAKE = make -C

include make.config

DEFINES= 

SUBDIRS = 


LIBS =  -lm
INCLUDES = -I. 

CFLAGS = -fPIC $(INCLUDES) $(DEFINES) -Wall


SRCS =  tkDial.c tkStripchart.c tkPie.c tkBargraph.c MeterCmds.c tkTurndial.c

OBJS =  tkDial.o tkStripchart.o tkPie.o tkBargraph.o MeterCmds.o tkTurndial.o


all: libmeters.so 
	
libmeters.so: $(OBJS)
	$(CC) -shared $(CFLAGS) $(OBJS) -o $@


$(MODULE): $(OBJS)
	ld -r -o $(MODULE) $(OBJS)
	cp $(MODULE) $(IBSH_HOME)/modules

clean: 
	-rm *.o *.so make.config
	set -e;for i in $(SUBDIRS); do make -C $$i clean; done

rcsput:
	ci -u $(SRCS) *.h

rcsget:
	for i in  ./RCS/*,v ; do co -l `basename $$i ,v` ;done


extensions: 
	set -e;for i in $(SUBDIRS); do make -C  $$i ; done

make.config:
	../config.tcl >make.config

install: all
	install libmeters.so $(INSTALLDIR)
	

	







