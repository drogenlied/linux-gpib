######################################################################
#
#   Makefile for NI4882(tm) compatible GPIB - C language Interface
#
#   (c) C.Schroeter 1994 
#
#######################################################################

export 
TARGET_ROOT = /usr/local

BIN_TARGET_DIR = $(TARGET_ROOT)/bin
LIB_TARGET_DIR = $(TARGET_ROOT)/lib
INC_TARGET_DIR = $(TARGET_ROOT)/include

LIB_CONFIG_FILE = /etc/gpib.conf


LIBNAME=libgpib.so.3.1.9

EXPORTED_INCLUDES = ib.h ibConf.h ibP.h
EXP_INC = $(EXPORTED_INCLUDES:%=$(ROOT)/include/%)
CWD := `pwd`

MAKE = make -C

######################################################################

export JUMP_DIR=./jump-dir

CFLAGS= -Wall -O2 -I. -I../include -I../driver/include -DDEFAULT_CONFIG_FILE=\"$(LIB_CONFIG_FILE)\"  \
	-D_REENTRANT -fpic
LDFLAGS = -L.

SRCS =  ibCac.c ibClr.c ibCmd.c ibConfYacc.c ibConfLex.c  ibEos.c ibEot.c \
	ibFind.c ibLines.c ibOnl.c ibPad.c ibRd.c ibRpp.c ibRsp.c ibRsv.c \
	ibSad.c ibSic.c ibSre.c ibTmo.c ibTrg.c ibWait.c ibWrt.c ibBdChrConfig.c\
	ibGts.c ibBoard.c ibError.c ibutil.c globals.c ibask.c ibppc.c \
	ibLoc.c ibDma.c ibdev.c ibbna.c async.c ibconfig.c ibFindLstn.c \
	ibEvent.c local_lockout.c self_test.c


OBJS =  $(SRCS:.c=.o) 


all: $(EXP_INC) libgpib.so 

libgpib.so:   $(OBJS) 
	ld --version-script=gpib_version_script -shared -lpthread $(OBJS) -o libgpib.so	

ibConfLex.c:  ibConfLex.l ibConfYacc.h 
	flex -Pgpib_yy -o$@ $<
	

ibConfYacc.c + ibConfYacc.h: ibConfYacc.y
	bison -d -y -p gpib_yy -o ibConfYacc.c ibConfYacc.y

clean: 
	$(RM) $(EXP_INC) *.o ibConfYacc.c ibConfLex.c *.a .depend ibConfYacc.h *.so


realclean : clean
	$(RM) __.SYMDEF lib*.so.* *.a *.done verify.out *.sa size.nm


install:
	$(INSTALL) -D libgpib.so $(LIB_TARGET_DIR)/$(LIBNAME)
	cd $(LIB_TARGET_DIR) && ln -sf $(LIBNAME) libgpib.so
	ldconfig $(LIB_TARGET_DIR)		
#	install ib.h           $(INC_TARGET_DIR)		
#        install ibConf.h       $(INC_TARGET_DIR)		

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
	$(DEPEND) $(CFLAGS) -M $(SRCS)  >./.depend

-include .depend
endif

include $(ROOT)/makefile.inc



