#
# Makefile for directory driver/tnt4882
#
include $(ROOT)/util/.config

SRCS =  tnt4882_init.c interrupt.c

OBJS =  $(SRCS:.c=.o)

INCLUDES = -I. -I../include -I../../include
CFLAGS += $(INCLUDES) $(OPTIONS)

all: tnt4882.o


tnt4882.o: $(OBJS)
	$(RLINK) -o tnt4882.o $(OBJS)


clean:
	$(RM) *.o .depend

install:
	$(INSTALL) -D tnt4882.o $(MODLIB)/misc/tnt4882.o


include $(ROOT)/makefile.inc

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(DEPEND) $(CFLAGS) -M $(SRCS)  >./.depend

-include .depend
endif


