######################################################################
#
#   Makefile for Example files
#
#   (c) C.Schroeter 1994          
#
#######################################################################

LIBNAME=../lib/libgpib.a

CFLAGS= -I. -I../include -I ../driver/include -Wall -O2
LDFLAGS = -L../lib

SRCS = ibtest.c
OBJS = ibtest.o



all:  depend ibtest rgpib_test   raw_access vfs_access


ibtest:	$(LIBNAME) ibtest.o
	$(CC) -o ibtest  ibtest.o $(LDFLAGS) -lgpib -lfl


rgpib_test:	$(LIBNAME) rgpib_test.o
	$(CC) -o rgpib_test  rgpib_test.o $(LDFLAGS) -lgpib -lfl

raw_access:	$(LIBNAME) raw_access.o
	$(CC) -o raw_access  raw_access.o $(LDFLAGS) -lgpib -lfl


vfs_access: vfs_access.o
	$(CC) -o vfs_access vfs_access.o

clean: 
	rm -f *.o ibtest rgpib_test raw_access vfs_access .depend

depend : $(SRCS)
	$(CPP) $(CFLAGS) -M $(SRCS)  >./.depend

install:
	@echo Install in examples

-include .depend










