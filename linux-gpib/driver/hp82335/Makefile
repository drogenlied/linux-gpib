#
# Makefile for directory driver/tnt4882-tms
#
include $(ROOT)/util/.config

SRCS =  hp82335_init.c 

OBJS =  $(SRCS:.c=.o)

all: hp82335.o


hp82335.o: $(OBJS)
	$(RLINK) -o hp82335.o $(OBJS)


clean:
	$(RM) *.o .depend

install:
	$(INSTALL) -D hp82335.o $(MODLIB)/misc/hp82335.o


include $(ROOT)/makefile.inc

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(DEPEND) $(CFLAGS) -MM $(SRCS)  >./.depend

-include .depend
endif


