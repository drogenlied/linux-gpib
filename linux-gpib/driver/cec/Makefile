#
# Makefile for directory driver/cec
#

SRCS =  init.c interrupt.c

OBJS =  $(SRCS:.c=.o)

all: cec-gpib.o

cec-gpib.o: $(OBJS)
	$(LD) -r -o cec-gpib.o $(OBJS)

clean:
	$(RM) *.o .depend

distclean: clean

install:
	$(INSTALL) -D cec-gpib.o $(MODLIB)/misc/cec-gpib.o

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(CC) $(CFLAGS) -MM $(SRCS)  >./.depend

-include .depend
endif


