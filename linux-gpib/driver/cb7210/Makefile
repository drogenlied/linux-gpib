#
# Makefile for directory driver/cb7210
#

SRCS =  cb7210_init.c cb7210_cs.c interrupt.c cb7210_aux.c cb7210_read.c cb7210_write.c

OBJS =  $(SRCS:.c=.o)

all: cb7210.o

cb7210.o: $(OBJS)
	$(LD) -r -o cb7210.o $(OBJS)

clean:
	$(RM) *.o .depend

distclean: clean

install:
	$(INSTALL) -D cb7210.o $(MODLIB)/misc/cb7210.o

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(CC) $(CFLAGS) -MM $(SRCS)  >./.depend

-include .depend
endif


