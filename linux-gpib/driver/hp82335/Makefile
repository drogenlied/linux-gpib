#
# Makefile for directory driver/atgpib
#

.SILENT:

ROOT=../..

SRCS =  adjcount.c cmd.c dma.c init.c lines.c read.c util.c \
	wait.c write.c aux.c io.c
OBJS =  $(SRCS:.c=.o)

INCLUDES = -I/usr/src/linux/include  -I. -I../include -I../../include
CFLAGS = $(INCLUDES) -D__KERNEL__ -DLINUX -O6 -m486 $(OPTIONS) -Wall -DMODULE -DMODULE -DMODVERSIONS -include /usr/src/linux/include/linux/modversions.h



all: board.o

board.o: $(OBJS)
	$(RLINK) -o board.o $(OBJS)


clean:
	$(RM) *.o .depend


include $(ROOT)/makefile.inc

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(DEPEND) $(CFLAGS) -M $(SRCS)  >./.depend

-include .depend
endif


