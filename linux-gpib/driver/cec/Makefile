#
# Makefile for directory driver/cec
#
include $(ROOT)/util/.config

SRCS =  init.c interrupt.c

OBJS =  $(SRCS:.c=.o)

all: cec-gpib.o

cec-gpib.o: $(OBJS)
	$(LD) -r -o cec-gpib.o $(OBJS)


clean:
	$(RM) *.o .depend

install:
	$(INSTALL) -D cec-gpib.o $(MODLIB)/misc/cec-gpib.o


include $(ROOT)/makefile.inc

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(DEPEND) $(CFLAGS) -MM $(SRCS)  >./.depend

-include .depend
endif


