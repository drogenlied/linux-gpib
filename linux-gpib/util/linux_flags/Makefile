#get flags for kernel modules from linux kernel source tree 

all:
	$(MAKE) -C $(LINUXDIR) SUBDIRS=$(PWD) modules

modules:
	echo ARCH=$(ARCH) >flags
	echo AFLAGS="$(AFLAGS)" >>flags
	echo LDFLAGS="$(LDFLAGS)" >>flags
	echo ARFLAGS="$(ARFLAGS)" >>flags
	echo CROSS_COMPILE="$(CROSS_COMPILE)" >>flags
	echo KERNELRELEASE="$(KERNELRELEASE)" >>flags
	echo CFLAGS="$(CFLAGS)" >>flags
	echo CC="$(CC)" >>flags
	echo LD="$(LD)" >>flags
	echo AS="$(AS)" >>flags
	echo RLINK="$(RLINK)" >>flags
	echo MODLIB="$(MODLIB)" >>flags

