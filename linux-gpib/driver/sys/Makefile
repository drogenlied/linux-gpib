#
# Makefile for directory driver/sys
#

-include $(ROOT)/util/.config

SRCS =  osfuncs.c  osinit.c  ostimer.c \
	osutil.c osmem.c
OBJS =  $(SRCS:.c=.o)

INCLUDES = -I. -I../include -I ../board/ -I../../include 

CFLAGS += $(INCLUDES) -DEXPORT_SYMTAB

all: sys.o

sys.o:  $(OBJS)
	$(RLINK) -o sys.o $(OBJS)

clean:
	rm -f *.o .depend

include $(ROOT)/makefile.inc

depend:

ifndef NODEPS
.depend: $(SRCS)
	$(DEPEND) $(CFLAGS) -M $(SRCS)  >./.depend

-include .depend
endif




