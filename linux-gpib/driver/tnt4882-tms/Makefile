#
# Makefile for directory driver/tnt4882-tms
#

SRCS =  init.c mite.c interrupt.c

OBJS =  $(SRCS:.c=.o)

all: tnt4882-tms.o

tnt4882-tms.o: $(OBJS)
	$(LD) -r -o tnt4882-tms.o $(OBJS)

clean:
	$(RM) *.o .depend

distclean: clean

install:
	$(INSTALL) -D tnt4882-tms.o $(MODLIB)/misc/tnt4882-tms.o

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(CC) $(CFLAGS) -MM $(SRCS)  >./.depend

-include .depend
endif


