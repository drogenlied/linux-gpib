
obj-m += ines_gpib.o

list-multi := ines_gpib.o

ines_gpib-objs := ines_init.o ines_cs.o interrupt.o ines_util.o ines_read.o ines_write.o

ines_gpib.o: $(ines_gpib-objs)
	$(LD) -r -o ines_gpib.o $(ines_gpib-objs)

include $(TOPDIR)/Rules.make
