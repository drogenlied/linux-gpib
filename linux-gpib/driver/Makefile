#######################################################################
#
#  Makefile for linux-gpib/driver 
#  
#   
#
#
#
########################################################################

-include $(ROOT)/util/.config
-include $(ROOT)/util/linux_flags/flags

EXPORTED_INCLUDES = gpib_types.h gpib_registers.h gpib_ioctl.h gpib_user.h 
EXP_INC = $(EXPORTED_INCLUDES:%=$(ROOT)/include/%)
CWD := `pwd`
CFLAGS := -I $(ROOT)/driver/include -I $(ROOT)/include $(CFLAGS)
MAKE = make OPTIONS="$(OPTIONS) " -C

SUBDIRS = nec7210 tms9914 sys
ifdef CB7210
SUBDIRS += cb7210
endif
ifdef INES
SUBDIRS += ines
endif
ifdef TNT4882
SUBDIRS += tnt4882
endif
ifdef PC2
SUBDIRS += pc2
endif
ifdef TNT4882TMS
SUBDIRS += tnt4882-tms
endif
ifdef CEC
SUBDIRS += cec
endif
ifdef HP82335
SUBDIRS += hp82335
endif

all : $(EXP_INC) subdirs 

subdirs:
	set -e; \
	for i in $(SUBDIRS); do \
		$(MAKE) $$i all ; \
	done	

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
	/sbin/insmod gpib-common.o 
	/sbin/insmod nec7210/nec7210.o 

unload:
	-/sbin/rmmod nec7210
	-/sbin/rmmod gpib-common 

test:  unload load run-test unload

install: 
	for i in $(SUBDIRS); do \
		$(MAKE) $$i install; \
	done	
	depmod -ae $(KERNELRELEASE)


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

export





