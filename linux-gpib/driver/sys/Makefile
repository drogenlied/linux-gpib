#
# Makefile for directory driver/sys
#

.SILENT:


ROOT=../..


SRCS =  osadjcnt.c osdma.c osfuncs.c  osinit.c  osinterrupt.c ostimer.c \
	osutil.c oslock.c osmem.c
OBJS =  $(SRCS:.c=.o)

INCLUDES = -I. -I../include -I../../include


CFLAGS = $(INCLUDES)  -D__KERNEL__ -DLINUX -O6 -m486 $(OPTIONS) -Wall -DMODULE
	


all: sys.o


sys.o:  $(OBJS)
	$(RLINK) -o sys.o $(OBJS)


clean:
	$(RM) *.o .depend 


include $(ROOT)/makefile.inc


depend:

ifndef NODEPS
.depend: $(SRCS)
	$(DEPEND) $(CFLAGS) -M $(SRCS)  >./.depend

-include .depend
endif




