######################################################################
#
#   Makefile for Example files
#
#   (c) C.Schroeter 1994
#   (c) Frank Mori Hess 2002,2003
#
#######################################################################

CFLAGS= -I. -I../include -I ../driver/include -Wall -D_REENTRANT -ansi -pedantic
ifdef GPIB_CONFIG_LIB_DEBUG
CFLAGS += -g3 -O
else
CFLAGS += -O2
endif
LDFLAGS = -L../lib -lgpib -lpthread

BINS = ibtest master_write_from_file slave_read_to_file master_read_to_file \
	slave_write_from_file
SRCS = $(BINS:%=%.c)

all: $(BINS) .depend

$(BINS) : % : %.o
	$(CC) -o $@ $< $(LDFLAGS)

clean:
	rm -f *.o $(BINS) .depend

.depend : $(SRCS)
	$(CPP) $(CFLAGS) -MM $(SRCS)  >./.depend

install:
	@echo Install in examples

-include .depend










