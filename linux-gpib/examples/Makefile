######################################################################
#
#   Makefile for Example files
#
#   (c) C.Schroeter 1994
#
#######################################################################

CFLAGS= -I. -I../include -I ../driver/include -Wall -O2 -D_REENTRANT
LDFLAGS = -L../lib

SRCS = ibtest.c master_write_from_file.c slave_read_to_file.c

all:  .depend ibtest master_write_from_file slave_read_to_file

ibtest:	ibtest.o
	$(CC) -o ibtest  ibtest.o $(LDFLAGS) -lgpib

master_write_from_file:	master_write_from_file.o
	$(CC) -o master_write_from_file master_write_from_file.o $(LDFLAGS) -lgpib

slave_read_to_file:	slave_read_to_file.o
	$(CC) -o slave_read_to_file slave_read_to_file.o $(LDFLAGS) -lgpib

clean:
	rm -f *.o ibtest master_write_from_file slave_read_to_file .depend

.depend : $(SRCS)
	$(CPP) $(CFLAGS) -M $(SRCS)  >./.depend

install:
	@echo Install in examples

-include .depend










