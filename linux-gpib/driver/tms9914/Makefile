#
# Makefile for directory driver/tms9914
#

CFLAGS += -DEXPORT_SYMTAB

SRCS =  cmd.c init.c read.c util.c \
	write.c aux.c interrupt.c

OBJS =  $(SRCS:.c=.o)

all: tms9914.o

tms9914.o: $(OBJS)
	$(LD) -r -o tms9914.o $(OBJS)

clean:
	$(RM) *.o .depend

install:
	$(INSTALL) -D tms9914.o $(MODLIB)/misc/tms9914.o

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(CC) $(CFLAGS) -MM $(SRCS)  >./.depend

-include .depend
endif


