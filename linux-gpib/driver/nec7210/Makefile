#
# Makefile for directory driver/pcIIa
#

SRCS =  cmd.c init.c read.c util.c \
	write.c aux.c interrupt.c

OBJS =  $(SRCS:.c=.o)

CFLAGS += -DEXPORT_SYMTAB

all: nec7210.o

nec7210.o: $(OBJS)
	$(RLINK) -o nec7210.o $(OBJS)

clean:
	$(RM) *.o .depend

install:
	$(INSTALL) -D nec7210.o $(MODLIB)/misc/nec7210.o

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(DEPEND) $(CFLAGS) -MM $(SRCS)  >./.depend

-include .depend
endif


