#
# Makefile for directory driver/pcIIa
#
include $(ROOT)/util/.config

SRCS =  cmd.c init.c pc2_init.c cb7210_init.c read.c util.c \
	write.c aux.c ines_init.c ines_cs.c tnt4882_init.c \
	interrupt.c cb7210_cs.c

OBJS =  $(SRCS:.c=.o)

INCLUDES = -I. -I../include -I../../include
CFLAGS += $(INCLUDES) $(OPTIONS)


all: nec7210.o

nec7210.o: $(OBJS)
	$(RLINK) -o nec7210.o $(OBJS)


clean:
	$(RM) *.o .depend


include $(ROOT)/makefile.inc

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(DEPEND) $(CFLAGS) -M $(SRCS)  >./.depend

-include .depend
endif


