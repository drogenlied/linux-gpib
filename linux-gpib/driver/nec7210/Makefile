#
# Makefile for directory driver/pcIIa
#
include $(ROOT)/util/.config

SRCS =  cmd.c init.c read.c util.c \
	write.c aux.c gpib_pci.c ines_pci.c ines_cs.c \
	interrupt.c

OBJS =  $(SRCS:.c=.o)

INCLUDES = -I. -I../include -I../../include
CFLAGS += $(INCLUDES) $(OPTIONS)


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


