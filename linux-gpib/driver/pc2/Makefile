#
# Makefile for directory driver/pc2
#
include $(ROOT)/util/.config

SRCS =  pc2_init.c interrupt.c

OBJS =  $(SRCS:.c=.o)

INCLUDES = -I. -I../include -I../../include
CFLAGS += $(INCLUDES) $(OPTIONS)

all: pc2-gpib.o


pc2-gpib.o: $(OBJS)
	$(RLINK) -o pc2-gpib.o $(OBJS)


clean:
	$(RM) *.o .depend

install:
	$(INSTALL) -D pc2-gpib.o $(MODLIB)/misc/pc2-gpib.o


include $(ROOT)/makefile.inc

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(DEPEND) $(CFLAGS) -M $(SRCS)  >./.depend

-include .depend
endif


