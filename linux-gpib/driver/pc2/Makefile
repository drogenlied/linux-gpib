#
# Makefile for directory driver/pc2
#

SRCS =  pc2_init.c interrupt.c

OBJS =  $(SRCS:.c=.o)

all: pc2-gpib.o

pc2-gpib.o: $(OBJS)
	$(LD) -r -o pc2-gpib.o $(OBJS)

clean:
	$(RM) *.o .depend

distclean: clean

install:
	$(INSTALL) -D pc2-gpib.o $(MODLIB)/misc/pc2-gpib.o

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(CC) $(CFLAGS) -MM $(SRCS)  >./.depend

-include .depend
endif


