######################################################################
#
#   Makefile for the gpib guile extension
#   (C) Stefan Jahn <stefan@lkcc.org>
#
#####################################################################

MAKE 		= make -C
ROOT 		= ../..
GPIBLIBS 	= -L$(ROOT)/lib -lgpib -lpthread
GPIBINCLUDE 	= -I$(ROOT)/include 

GUILEDIR 	= /usr/local
GUILELIBS 	= -L$(GUILEDIR)/lib -lguile
GUILEINCLUDE    = -I$(GUILEDIR)/include

INSTALLDIR = /usr/local/lib
LIBS =  $(GPIBLIBS) $(GUILELIBS)
INCLUDES = -I. -I$(ROOT)/include -I$(ROOT)/driver/include $(GUILEINCLUDE)
CFLAGS = -fpic $(INCLUDES) $(DEFINES) -Wall


SRCS = gpib.c

OBJS = gpib.o


all: libgpib-guile.so

libgpib-guile.so: $(OBJS)
	$(CC) -shared $(OBJS) $(LIBS) -o $@

clean: 
	rm -f *.o *.so make.config *~

realclean: 
	rm -f *.o

install:
	install -D ./libgpib-guile.so $(INSTALLDIR)/libgpib-guile.so
	ldconfig $(INSTALLDIR)
