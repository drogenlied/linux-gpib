#
# Makefile for directory driver/tnt4882-tms
#

SRCS =  hp82335_init.c

OBJS =  $(SRCS:.c=.o)

all: hp82335.o

hp82335.o: $(OBJS)
	$(LD) -r -o hp82335.o $(OBJS)

clean:
	$(RM) *.o .depend

distclean: clean

install:
	$(INSTALL) -D hp82335.o $(MODLIB)/misc/hp82335.o

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(CC) $(CFLAGS) -MM $(SRCS)  >./.depend

-include .depend
endif


