#
# Makefile for directory driver/sys
#

CFLAGS += -DEXPORT_SYMTAB

SRCS =  osfuncs.c  osinit.c  ostimer.c osutil.c autopoll.c ibcac.c ibcmd.c \
	ibgts.c ibinit.c iblines.c ibread.c ibrpp.c ibrsv.c ibsic.c \
        ibsre.c ibutil.c ibwait.c ibwrite.c device.c event.c

OBJS =  $(SRCS:.c=.o)

all: gpib-common.o

gpib-common.o:  $(OBJS)
	$(LD) -r -o gpib-common.o $(OBJS)

clean:
	rm -f *.o .depend

distclean: clean

install:
	$(INSTALL) -D gpib-common.o $(MODLIB)/misc/gpib-common.o

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(CC) $(CFLAGS) -MM $(SRCS)  >./.depend

-include .depend
endif




