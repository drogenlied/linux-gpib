#
# Makefile for directory driver/pcIIa
#

CFLAGS += -DEXPORT_SYMTAB

SRCS =  cmd.c init.c read.c util.c \
	write.c aux.c interrupt.c

OBJS =  $(SRCS:.c=.o)

all: nec7210.o

nec7210.o: $(OBJS)
	$(LD) -r -o nec7210.o $(OBJS)

clean:
	$(RM) *.o .depend

distclean: clean

install:
	$(INSTALL) -D nec7210.o $(MODLIB)/misc/nec7210.o

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(CC) $(CFLAGS) -MM $(SRCS)  >./.depend

-include .depend
endif


