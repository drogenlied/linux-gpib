#
# Makefile for directory driver/cb7210
#
include $(ROOT)/util/.config

SRCS =  ines_init.c ines_cs.c interrupt.c

OBJS =  $(SRCS:.c=.o)

all: ines-gpib.o

ines-gpib.o: $(OBJS)
	$(RLINK) -o ines-gpib.o $(OBJS)

clean:
	$(RM) *.o .depend

install:
	$(INSTALL) -D ines-gpib.o $(MODLIB)/misc/ines-gpib.o


include $(ROOT)/makefile.inc

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(DEPEND) $(CFLAGS) -M $(SRCS)  >./.depend

-include .depend
endif


