#
# Makefile for directory driver/tms9914
#
include $(ROOT)/util/.config

SRCS =  cmd.c init.c read.c util.c \
	write.c aux.c interrupt.c 

OBJS =  $(SRCS:.c=.o)

CFLAGS += -DEXPORT_SYMTAB

all: tms9914.o


tms9914.o: $(OBJS)
	$(RLINK) -o tms9914.o $(OBJS)


clean:
	$(RM) *.o .depend

install:
	$(INSTALL) -D tms9914.o $(MODLIB)/misc/tms9914.o


include $(ROOT)/makefile.inc

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(DEPEND) $(CFLAGS) -MM $(SRCS)  >./.depend

-include .depend
endif


