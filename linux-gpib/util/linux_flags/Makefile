#get flags for kernel modules from linux kernel source tree

all:
	$(MAKE) -C $(LINUXDIR) SUBDIRS=$(PWD) modules

modules:
	echo "#compilation variables from kernel source" >flags
	echo ARCH_LINUX=$(ARCH) >>flags
	echo AFLAGS_LINUX="$(AFLAGS)" >>flags
	echo LDFLAGS_LINUX="$(LDFLAGS)" >>flags
	echo ARFLAGS_LINUX="$(ARFLAGS)" >>flags
	echo CROSS_COMPILE_LINUX="$(CROSS_COMPILE)" >>flags
	echo KERNELRELEASE_LINUX="$(KERNELRELEASE)" >>flags
	echo CFLAGS_LINUX="$(CFLAGS)" >>flags
	echo CC_LINUX="$(CC)" >>flags
	echo LD_LINUX="$(LD)" >>flags
	echo AS_LINUX="$(AS)" >>flags
	echo MODLIB_LINUX="$(MODLIB)" >>flags
	echo >> flags
	
clean:
	rm -f flags

