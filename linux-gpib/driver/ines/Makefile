#
# Makefile for directory driver/cb7210
#

SRCS =  ines_init.c ines_cs.c interrupt.c ines_util.c ines_read.c ines_write.c

OBJS =  $(SRCS:.c=.o)

all: ines-gpib.o

ines-gpib.o: $(OBJS)
	$(LD) -r -o ines-gpib.o $(OBJS)

clean:
	$(RM) *.o .depend

distclean: clean

install:
	$(INSTALL) -D ines-gpib.o $(MODLIB)/misc/ines-gpib.o

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(CC) $(CFLAGS) -MM $(SRCS)  >./.depend

-include .depend
endif


