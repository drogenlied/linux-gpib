#
# Makefile for directory driver/atgpib
#

SRCS =  adjcount.c cmd.c dma.c init.c lines.c read.c util.c \
	wait.c write.c aux.c interrupt.c
OBJS =  $(SRCS:.c=.o)

INCLUDES = -I. -I../include -I../../include -I../sys 
CFLAGS += $(INCLUDES) $(OPTIONS)

all: board.o

board.o: $(OBJS)
	$(RLINK) -o board.o $(OBJS)

clean:
	$(RM) *.o .depend

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(DEPEND) $(CFLAGS) -M $(SRCS)  >./.depend

-include .depend
endif


