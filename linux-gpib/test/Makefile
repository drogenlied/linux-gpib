######################################################################
#
#   Makefile for test files
#
#   (c) C.Schroeter 1994
#   (c) Frank Mori Hess 2002,2003
#
#######################################################################

-include $(ROOT)/util/.config

CFLAGS= -I. -I../include -I ../driver/include -Wall -D_REENTRANT -g3 -O 
LDFLAGS = -L../lib -lgpib -lpthread

BINS = libgpib_test
SRCS = $(BINS:%=%.c)

all: $(BINS) .depend

$(BINS) : % : %.o
	$(CC) -o $@ $< $(LDFLAGS)

clean:
	rm -f *.o $(BINS) .depend

.depend : $(SRCS)
	$(CPP) $(CFLAGS) -MM $(SRCS)  >./.depend

install:
	@echo Install in test

-include .depend










