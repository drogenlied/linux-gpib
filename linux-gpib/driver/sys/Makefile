
obj-m += gpib_common.o

list-multi := gpib_common.o

gpib_common-objs := osfuncs.o  osinit.o  ostimer.o osutil.o autopoll.o ibcac.o ibcmd.o \
	ibgts.o ibinit.o iblines.o ibread.o ibrpp.o ibrsv.o ibsic.o \
	ibsre.o ibutil.o ibwait.o ibwrite.o device.o event.o

export-objs := $(gpib_common-objs)

gpib_common.o: $(gpib_common-objs)
	$(LD) -r -o gpib_common.o $(gpib_common-objs)
