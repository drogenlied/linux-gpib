#
# Makefile for directory driver/atgpib
#

SRCS =  cmd.c init.c lines.c read.c util.c \
	wait.c write.c aux.c gpib_cs.c gpib_pci.c interrupt.c
OBJS =  $(SRCS:.c=.o)

INCLUDES = -I. -I../include -I../../include -I../sys 
CFLAGS += $(INCLUDES)



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


