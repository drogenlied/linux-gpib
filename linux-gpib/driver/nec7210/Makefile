
obj-m += nec7210.o

list-multi := nec7210.o

nec7210-objs := cmd.o init.o read.o util.o write.o aux.o interrupt.o

export-objs := $(nec7210-objs)

nec7210.o: $(nec7210-objs)
	$(LD) -r -o nec7210.o $(nec7210-objs)

include $(TOPDIR)/Rules.make
