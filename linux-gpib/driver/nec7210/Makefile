#
# Makefile for directory driver/pcIIa
#
include $(ROOT)/util/.config

SRCS =  cmd.c init.c read.c util.c \
	write.c aux.c interrupt.c 

OBJS =  $(SRCS:.c=.o)

INCLUDES = -I. -I../include -I../../include
CFLAGS += $(INCLUDES) $(OPTIONS) -DEXPORT_SYMTAB

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


