#
# Makefile for directory driver/protocol
#

-include $(ROOT)/util/.config

SRCS = ibcac.c ibcmd.c ibgts.c ibinit.c iblines.c ibread.c ibrpp.c ibrsv.c ibsic.c \
	ibsre.c ibutil.c ibwait.c ibwrite.c device.c ibAutoSP.c

OBJS = $(SRCS:.c=.o)


INCLUDES = -I. -I../include -I../../include -I$(LINUXDIR)/include

CFLAGS += $(INCLUDES) $(OPTIONS) 

all: ieee488.o

ieee488.o : $(OBJS)
	$(RLINK) -o ieee488.o $(OBJS)


clean:
	rm -f *.o .depend


include $(ROOT)/makefile.inc

install:

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(DEPEND) $(CFLAGS) -M $(SRCS)  >./.depend

-include .depend
endif













