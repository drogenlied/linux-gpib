#
# Makefile for directory driver/tnt4882
#

SRCS =  tnt4882_init.c mite.c interrupt.c tnt4882_cs.c tnt4882_aux.c \
	tnt4882_write.c tnt4882_read.c

OBJS =  $(SRCS:.c=.o)

all: tnt4882.o

tnt4882.o: $(OBJS)
	$(LD) -r -o tnt4882.o $(OBJS)

clean:
	$(RM) *.o .depend

install:
	$(INSTALL) -D tnt4882.o $(MODLIB)/misc/tnt4882.o

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(DEPEND) $(CFLAGS) -MM $(SRCS)  >./.depend

-include .depend
endif


