######################################################################
#
#   Makefile for NI4882(tm) compatible GPIB - C language Interface
#
#   (c) C.Schroeter 1994 
#
#######################################################################

export ROOT
TARGET_ROOT = /usr/local

SUBDIRS = net

BIN_TARGET_DIR = $(TARGET_ROOT)/bin
LIB_TARGET_DIR = $(TARGET_ROOT)/lib
INC_TARGET_DIR = $(TARGET_ROOT)/include

LIB_CONFIG_FILE = /etc/gpib.conf


LIBNAME=libgpib.a
LIB_NAME=libgpib

EXPORTED_INCLUDES = ib.h ibProto.h ibConf.h ibP.h
EXP_INC = $(EXPORTED_INCLUDES:%=$(ROOT)/include/%)
CWD := `pwd`

MAKE = make -C

## Uncomment the desired line
# Build static library
DEFAULT_BUILD= $(LIBNAME)
# Build shared Library
#DEFAULT_BUILD= $(LIBNAME) shared

######################################################################

export JUMP_DIR=./jump-dir

CFLAGS= -I. -I../include -DDEFAULT_CONFIG_FILE=\"$(LIB_CONFIG_FILE)\" -g \
	$(SHARED_FLAGS)
LDFLAGS = -L.

SRCS =  ibCac.c ibClr.c ibCmd.c ibConfYacc.c ibConfLex.c  ibEos.c ibEot.c \
	ibFind.c ibLines.c ibOnl.c ibPad.c ibRd.c ibRpp.c ibRsp.c ibRsv.c \
	ibSad.c ibSic.c ibSre.c ibTmo.c ibTrg.c ibWait.c ibWrt.c ibBdChrConfig.c\
	ibOpt.c ibGts.c ibBoard.c ibError.c ibutil.c ibAPE.c ibSdbg.c \
	ibFlags.c ibLoc.c ibDma.c ibConfig.c \
	ibFindLstn.c #ibEvent.c


OBJS =  $(SRCS:.c=.o) net/rgpib.o



all: $(EXP_INC) subdirs $(DEFAULT_BUILD) server

subdirs:
	set -e; \
	for i in $(SUBDIRS); do \
		$(MAKE) $$i all ; \
	done

server:
	make -C net rgpib_svc

$(LIBNAME):   $(OBJS) 
	rm -f $(LIBNAME)
	ar cr $(LIBNAME) $(OBJS)
	ranlib $(LIBNAME)

ibConfLex.c:  ibConfLex.l y.tab.h
	$(LEX) -o$@ $<
	sed s/yy/gpib_yy/g ibConfLex.c > dummy.c
	mv dummy.c ibConfLex.c
	sed s/gpib_gpib/gpib/g ibConfLex.c > dummy.c
	mv dummy.c ibConfLex.c
	sed s/gpib_yywrap/yywrap/g ibConfLex.c > dummy.c
	mv dummy.c ibConfLex.c
	sed s/YY/GPIB_YY/g ibConfLex.c > dummy.c
	mv dummy.c ibConfLex.c
	sed s/GPIB_GPIB/GPIB/g ibConfLex.c > dummy.c
	mv dummy.c ibConfLex.c
	

ibConfYacc.c + y.tab.h: ibConfYacc.y
	$(YACC) $<
	sed s/yy/gpib_yy/g y.tab.c > ibConfYacc.c
	sed s/gpib_gpib/gpib/g ibConfYacc.c > dummy.c
	mv dummy.c ibConfYacc.c
	sed s/YY/GPIB_YY/g ibConfYacc.c > dummy.c
	mv dummy.c ibConfYacc.c
	sed s/GPIB_GPIB/GPIB/g ibConfYacc.c > dummy.c
	mv dummy.c ibConfYacc.c
	rm -f y.tab.c
	sed s/yy/gpib_yy/g y.tab.h > dummy.h
	mv dummy.h y.tab.h
	sed s/gpib_gpib/gpib/g y.tab.h > dummy.h
	mv dummy.h y.tab.h
	sed s/YY/GPIB_YY/g y.tab.h > dummy.h
	mv dummy.h y.tab.h
	sed s/GPIB_GPIB/GPIB/g y.tab.h > dummy.h
	mv dummy.h y.tab.h


ibProto.h: $(SRCS)
	echo >ibProto.h
	echo "#ifdef __cplusplus" >>ibProto.h 
	echo "extern \"C\" {" >>ibProto.h 
	echo "#endif" >>ibProto.h 
	echo "/***** Public Functions ******/" >>ibProto.h
	grep -h PUBLIC *.c|/lib/cpp -P -DPUBLIC=extern -DVOID=void| sed 's/)/);/' >>ibProto.h
	echo "/***** Private Functions ******/" >>ibProto.h 
	grep -h PRIVATE *.c|/lib/cpp -P -DPRIVATE=extern -DVOID=void| sed 's/)/);/' >>ibProto.h
	grep -h _VARARGS *.c| awk '{print "extern "$$2" "$$3"();" }' >>ibProto.h
	echo "#ifdef __cplusplus" >>ibProto.h 
	echo "}" >>ibProto.h 
	echo "#endif" >>ibProto.h 






clean: 
	$(RM) $(EXP_INC) *.o ibConfYacc.c ibConfLex.c *.a .depend y.tab.h ibProto.h
	set -e; \
	for i in $(SUBDIRS); do \
		make -C $$i clean; \
	done


realclean : clean
	$(RM) __.SYMDEF lib*.so.* *.a *.done verify.out *.sa size.nm


install:
	$(INSTALL) $(LIBNAME)     $(LIB_TARGET_DIR)		
#	install ib.h           $(INC_TARGET_DIR)		
#        install ibConf.h       $(INC_TARGET_DIR)		

rcsput:
	ci -u $(SRCS) *.h *.l *.y  gpib.conf.dist

rcsget:
	for i in  ./RCS/*,v ; do co -l `basename $$i ,v` ;done
	

######################################################################
#
#  building rules for shared libraries
#
##
#
#  change SHD_ADR if needed
#
#  


SHD_ADR = 0x80000000
SHD_JTS = 0x4000
SHD_GOT = 4096
SHD_VER = 0.1.1
SHD_NAME= $(LIB_NAME)


shared: shared_1 shared_2

shared_1:
	make clean
	-rm ./jump-dir/* *.done
	-rm lib*.so.*
	-rm lib*.sa
	export JUMP_LIB=$(SHD_NAME)
	export JUMP_DIR=`pwd`/jump-dir
	make SHARED_FLAGS=-B/usr/dll/jump/ 
	mv ./jump-dir/jump.log ./jump-dir/jump.log~
	cat ./jump-dir/jump.log~ | sed 's/libxyzzy/$(SHD_NAME)/' >./jump-dir/jump.log
	rm ./jump-dir/jump.log~
	/usr/dll/bin/getvars
	/usr/dll/bin/getfuncs
	rm ./jump-dir/jump.log
	nm --no-cplus -o /usr/lib/libc.sa | grep __GOT_ | sed 's/ __GOT__/_/' >./jump-dir/jump.import
	touch shared_1.done

shared_2: shared_1.done
	make clean
	export JUMP_LIB=$(SHD_NAME)
	export JUMP_DIR=`pwd`/jump-dir
	make SHARED_FLAGS=-B/usr/dll/jump/ 
	/usr/dll/bin/getsize >./jump-dir/jump.vars-new
	mv ./jump-dir/jump.vars-new ./jump-dir/jump.vars
	/usr/dll/bin/mkimage -l $(SHD_NAME) -v $(SHD_VER) -a $(SHD_ADR) -j $(SHD_JTS) -g $(SHD_GOT) -- $(OBJS) -lgcc -lc 
	/usr/dll/bin/mkstubs -l $(SHD_NAME) -v $(SHD_VER) -a $(SHD_ADR) -j $(SHD_JTS) -g $(SHD_GOT) -- $(SHD_NAME)
	/usr/dll/bin/verify-shlib -l $(SHD_NAME).so.$(SHD_VER) $(SHD_NAME).sa
	touch shared_2.done
	
	
$(EXP_INC):
	rm -f $@
	ln -s $(CWD)/`basename $@` $@



depend:

ifndef NODEPS
.depend: ibProto.h $(SRCS) $(EXP_INC)
	$(DEPEND) $(CFLAGS) -M $(SRCS)  >./.depend

-include .depend
endif

include $(ROOT)/makefile.inc



