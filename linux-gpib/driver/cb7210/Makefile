#
# Makefile for directory driver/cb7210
#
include $(ROOT)/util/.config

SRCS =  cb7210_init.c cb7210_cs.c interrupt.c cb7210_aux.c

OBJS =  $(SRCS:.c=.o)

all: cb7210.o


cb7210.o: $(OBJS)
	$(RLINK) -o cb7210.o $(OBJS)


clean:
	$(RM) *.o .depend

install:
	$(INSTALL) -D cb7210.o $(MODLIB)/misc/cb7210.o


include $(ROOT)/makefile.inc

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(DEPEND) $(CFLAGS) -MM $(SRCS)  >./.depend

-include .depend
endif


