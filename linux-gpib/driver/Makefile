#######################################################################
#
#  Makefile for linux-gpib/driver 
#  
#   
#
#
#
########################################################################
# Choose a unique Major number for your device look at
# ~/linux/include/linux/majors.h
#
DRIVER_DEVICE_ID = 0

DEBUG_LEVEL=0x3d

export 

-include $(ROOT)/util/.config
-include $(ROOT)/util/linux_flags/flags

########################################################################
#
#  Configuration Options
#
#  -DSLOW_IO if  you need to slow down bus i/o
#  -DREALLY_SLOW_IO (in addition to SLOW_IO) makes i/o slower
#
#  -DBUILD_DEBUG build version with built in debugging code that
#                helps to trace errors (the size of the module grows up to
#                7 pages)
#
#  -DUSE_DMA to enable DMA Transfer Mode
#######################################################################
#
#  Do not Edit below
#

EXPORTED_INCLUDES = gpib_types.h gpib_registers.h gpib_ioctl.h gpib_user.h 
EXP_INC = $(EXPORTED_INCLUDES:%=$(ROOT)/include/%)
CWD := `pwd`

OBJS = ./board/board.o

MAKE = make OPTIONS="$(OPTIONS) " -C
SUBDIRS = board protocol sys


all : $(EXP_INC) subdirs gpib$(DRIVER_DEVICE_ID).o gpib-common.o

subdirs:
	set -e; \
	for i in $(SUBDIRS); do \
		$(MAKE) $$i all ; \
	done	

gpib$(DRIVER_DEVICE_ID).o: $(OBJS)
	$(RLINK) -o gpib$(DRIVER_DEVICE_ID).o $(OBJS) 

gpib-common.o: ./sys/sys.o ./protocol/ieee488.o
	$(RLINK) -o gpib-common.o ./sys/sys.o ./protocol/ieee488.o

clean :
	rm -f *.o kernel_version.c $(EXP_INC) 
	set -e; \
	for i in $(SUBDIRS); do \
		test -d $$i && $(MAKE) $$i NODEPS=y clean ; \
	done


depend :
	set -e; \
	for i in $(SUBDIRS); do \
		$(MAKE) $$i depend ; \
	done

load:
	/sbin/insmod gpib$(DRIVER_DEVICE_ID).o 

dload:
	/sbin/insmod gpib$(DRIVER_DEVICE_ID).o dbgMask=$(DEBUG_LEVEL)

unload:
	-/sbin/rmmod gpib$(DRIVER_DEVICE_ID)

test:  unload load run-test unload


device:
	mknod /dev/gpib$(DRIVER_DEVICE_ID)/master c $(DRIVER_MAJOR) 0



install:
	$(INSTALL) -D gpib$(DRIVER_DEVICE_ID).o gpib-common.o $(MODLIB)/misc/
	depmod -a $(KERNELRELEASE)


# make module version matching kernel version
#kernel_version.c: 
#	@echo char kernel_version[] = \"`uname -r`\"\; > kernel_version.c






rcsput:
	ci -u $(SRCS) *.h

rcsget:
	for i in  ./RCS/*,v ; do co -l `basename $$i ,v` ;done


$(EXP_INC):
	rm -f $@
	ln -s $(CWD)/include/`basename $@` $@







