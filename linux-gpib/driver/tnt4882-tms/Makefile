#
# Makefile for directory driver/tnt4882-tms
#
include $(ROOT)/util/.config

SRCS =  init.c mite.c interrupt.c

OBJS =  $(SRCS:.c=.o)

all: tnt4882-tms.o

tnt4882-tms.o: $(OBJS)
	$(RLINK) -o tnt4882-tms.o $(OBJS)

clean:
	$(RM) *.o .depend

install:
	$(INSTALL) -D tnt4882-tms.o $(MODLIB)/misc/tnt4882-tms.o

include $(ROOT)/makefile.inc

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(DEPEND) $(CFLAGS) -MM $(SRCS)  >./.depend

-include .depend
endif


