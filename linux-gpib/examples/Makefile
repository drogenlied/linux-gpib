######################################################################
#
#   Makefile for Example files
#
#   (c) C.Schroeter 1994          
#
#######################################################################

CFLAGS= -I. -I../include -I ../driver/include -Wall -O2
LDFLAGS = -L../lib

SRCS = ibtest.c
OBJS = ibtest.o



all:  depend ibtest raw_access


ibtest:	ibtest.o
	$(CC) -o ibtest  ibtest.o $(LDFLAGS) -lgpib -lfl

raw_access: raw_access.o
	$(CC) -o raw_access  raw_access.o $(LDFLAGS) -lgpib -lfl


clean: 
	rm -f *.o ibtest raw_access .depend

depend : $(SRCS)
	$(CPP) $(CFLAGS) -M $(SRCS)  >./.depend

install:
	@echo Install in examples

-include .depend










