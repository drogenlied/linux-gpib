#
# Makefile for directory driver/protocol
#

.SILENT:

ROOT=../..

SRCS = ibcac.c ibcmd.c ibgts.c ibinit.c iblines.c ibread.c ibrpp.c ibrsv.c ibsic.c \
	ibsre.c ibutil.c ibwait.c ibwrite.c device.c ibAutoSP.c

OBJS = $(SRCS:.c=.o)


INCLUDES = -I. -I../include -I../../include

CFLAGS = $(INCLUDES)  -D__KERNEL__ -DLINUX -O6 -m486 $(OPTIONS) -Wall -DMODULE

all: ieee488.o

ieee488.o : $(OBJS)
	$(RLINK) -o ieee488.o $(OBJS)


clean:
	$(RM) *.o .depend


include $(ROOT)/makefile.inc

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(DEPEND) $(CFLAGS) -M $(SRCS)  >./.depend

-include .depend
endif













