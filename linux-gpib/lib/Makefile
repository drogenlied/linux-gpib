######################################################################
#
#   Makefile for Linux GPIB - C language Interface
#
#   Copyright (c) C.Schroeter 1994 
#   Copyright (c) Frank Mori Hess 2001,2002
#
#######################################################################
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
########################################################################

export 
TARGET_ROOT = /usr/local

BIN_TARGET_DIR = $(TARGET_ROOT)/bin
LIB_TARGET_DIR = $(TARGET_ROOT)/lib
INC_TARGET_DIR = $(TARGET_ROOT)/include

LIB_CONFIG_FILE = /etc/gpib.conf


LIBNAME=libgpib.so.$(GPIB_VERSION)

EXPORTED_INCLUDES = ib.h ibConf.h ibP.h
EXP_INC = $(EXPORTED_INCLUDES:%=$(ROOT)/include/%)
CWD := `pwd`

MAKE = make -C

######################################################################

include $(ROOT)/util/.config

export JUMP_DIR=./jump-dir

CPPFLAGS= -I. -I$(ROOT)/include -I$(ROOT)/driver/include -DDEFAULT_CONFIG_FILE=\"$(LIB_CONFIG_FILE)\"  \
	-D_REENTRANT -DPIC
CFLAGS= -Wall -fpic
ifdef GPIB_CONFIG_LIB_DEBUG
CFLAGS += -g3 -O
else
CFLAGS += -O2
endif

LDFLAGS = -L.

SRCS =  ibCac.c ibClr.c ibCmd.c ibConfYacc.c ibConfLex.c  ibEos.c ibEot.c \
	ibFind.c ibLines.c ibOnl.c ibPad.c ibRd.c ibRpp.c ibRsp.c ibRsv.c \
	ibSad.c ibSic.c ibSre.c ibTmo.c ibTrg.c ibWait.c ibWrt.c \
	ibGts.c ibBoard.c ibError.c ibutil.c globals.c ibask.c ibppc.c \
	ibLoc.c ibDma.c ibdev.c ibbna.c async.c ibconfig.c ibFindLstn.c \
	ibEvent.c local_lockout.c self_test.c pass_control.c ibstop.c


OBJS =  $(SRCS:.c=.o)


all: $(EXP_INC) libgpib.so gpib_config

libgpib.so:   $(OBJS)
	ld --version-script=gpib_version_script -shared -lpthread $(OBJS) -o libgpib.so

ibConfLex.c: ibConfLex.l ibConfYacc.h
	flex -Pgpib_yy -o$@ $<

ibConfYacc.c + ibConfYacc.h: ibConfYacc.y
	bison -d -y -p gpib_yy -o ibConfYacc.c ibConfYacc.y

.PHONY: gpib_config
gpib_config:
	make -C gpib_config all

clean:
	$(RM) $(EXP_INC) *.o *.a .depend *.so
# don't cleanup bison/flex files since I want to avoid bison bugs in older
# versions of bison that people may have installed
#	$(RM) ibConfYacc.c ibConfLex.c ibConfYacc.h
	make -C gpib_config clean

realclean : clean
	$(RM) __.SYMDEF lib*.so.* *.a *.done verify.out *.sa size.nm


install:
	$(INSTALL) -D libgpib.so $(LIB_TARGET_DIR)/$(LIBNAME)
	cd $(LIB_TARGET_DIR) && ln -sf $(LIBNAME) libgpib.so
	ldconfig $(LIB_TARGET_DIR)
	make -C gpib_config install

rcsput:
	ci -u $(SRCS) *.h *.l *.y  gpib.conf.dist

rcsget:
	for i in  ./RCS/*,v ; do co -l `basename $$i ,v` ;done


$(EXP_INC):
	rm -f $@
	ln -s $(CWD)/`basename $@` $@


depend:

ifndef NODEPS
.depend: $(SRCS) $(EXP_INC)
	$(DEPEND) $(CPPFLAGS) -MM $(SRCS)  >./.depend

-include .depend
endif

include $(ROOT)/makefile.inc

