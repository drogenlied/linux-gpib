######################################################################
#
#   Makefile for the gpib guile extension
#   Copyright (C) 2003 Stefan Jahn <stefan@lkcc.org>
#
#####################################################################

MAKE 		= make -C
ROOT 		= ../..
GPIBLIBS 	= -L$(ROOT)/lib -lgpib -lpthread
GPIBINCLUDE 	= -I$(ROOT)/include

GUILEDIR 	= /usr/local
GUILELIBS 	= -L$(GUILEDIR)/lib -lguile
GUILEINCLUDE    = -I$(GUILEDIR)/include

PREFIX = /usr/local
INSTALLDIR = $(PREFIX)/lib
LIBS =  $(GPIBLIBS) $(GUILELIBS)
INCLUDES = -I. -I$(ROOT)/include -I$(ROOT)/driver/include $(GUILEINCLUDE)
CFLAGS = -fpic $(INCLUDES) $(DEFINES) -Wall
CC = gcc -O2

SRCS = gpib.c

OBJS = gpib.o


all: libgpib-guile.so

libgpib-guile.so: $(OBJS)
	$(CC) -shared $(OBJS) $(LIBS) -o $@ -Wl,-rpath $(GUILEDIR)/lib -Wl,-rpath $(INSTALLDIR)

clean:
	rm -f *.o *.so *~

install:
	install -D ./libgpib-guile.so $(INSTALLDIR)/libgpib-guile.so
	ldconfig $(INSTALLDIR)
