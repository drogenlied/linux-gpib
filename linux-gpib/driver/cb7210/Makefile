
obj-m += cb7210.o

list-multi := cb7210.o

cb7210-objs := cb7210_init.o cb7210_cs.o interrupt.o cb7210_aux.o \
	cb7210_read.o cb7210_write.o

cb7210.o: $(cb7210-objs)
	$(LD) -r -o cb7210.o $(cb7210-objs)

include $(TOPDIR)/Rules.make
