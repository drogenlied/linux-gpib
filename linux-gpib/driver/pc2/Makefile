
obj-m += pc2_gpib.o

list-multi := pc2_gpib.o

pc2_gpib-objs := pc2_init.o interrupt.o

pc2_gpib.o: $(pc2_gpib-objs)
	$(LD) -r -o pc2_gpib.o $(pc2_gpib-objs)

include $(TOPDIR)/Rules.make
