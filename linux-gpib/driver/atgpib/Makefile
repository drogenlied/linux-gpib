#
# Makefile for directory driver/atgpib
#

include $(ROOT)/util/.config
 
SRCS =  adjcount.c cmd.c dma.c init.c lines.c read.c util.c \
	wait.c write.c aux.c 
OBJS =  $(SRCS:.c=.o)

INCLUDES = -I. -I../include -I../../include -I$(LINUXDIR)/include
CFLAGS = $(INCLUDES) -D__KERNEL__ -DLINUX -O2 -m486 $(OPTIONS) -Wall -DMODULE

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


