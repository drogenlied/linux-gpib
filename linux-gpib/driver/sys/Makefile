#
# Makefile for directory driver/sys
#

SRCS =  osfuncs.c  osinit.c  ostimer.c osutil.c autopoll.c ibcac.c ibcmd.c \
	ibgts.c ibinit.c iblines.c ibread.c ibrpp.c ibrsv.c ibsic.c \
        ibsre.c ibutil.c ibwait.c ibwrite.c device.c event.c

OBJS =  $(SRCS:.c=.o)

CFLAGS += -DEXPORT_SYMTAB

all: gpib-common.o

gpib-common.o:  $(OBJS)
	$(RLINK) -o gpib-common.o $(OBJS)

clean:
	rm -f *.o .depend

install:
	$(INSTALL) -D gpib-common.o $(MODLIB)/misc/gpib-common.o

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(DEPEND) $(CFLAGS) -MM $(SRCS)  >./.depend

-include .depend
endif




