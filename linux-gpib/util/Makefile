######################################################################
#
#   Makefile for GPIB utils
#   (C) C.Schroeter clausi@chemie.fu-berlin.de
#
#####################################################################
#  
#

BINDIR = /usr/local/bin


LIBS =  -L../lib -lgpib -lfl
INCLUDES = -I. -I../include  

CFLAGS = $(INCLUDES) $(DEFINES) -Wall -O2


######################################################################

SRCS = ibchk.c

OBJS = ibchk.o


all: ibchk ibdump


ibchk: $(OBJS) ../lib/libgpib.a
	$(CC) -o ibchk $(CFLAGS) $(OBJS) $(LIBS) 


DUMP_OBJS = ibdump.o ibDevMan.o

ibdump: $(DUMP_OBJS) ../lib/libgpib.a
	$(CC) -o ibdump $(CFLAGS) $(INCLUDES) $(DUMP_OBJS) $(LIBS) 

clean: 
	rm -f *.o ibchk

rcsput:
	ci -u $(SRCS) *.h

rcsget:
	for i in  ./RCS/*,v ; do co -l `basename $$i ,v` ;done


install: all
	install ./ibdump $(BINDIR)
	install ./ibchk $(BINDIR)
