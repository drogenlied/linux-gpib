
obj-m += hp82335.o

list-multi := hp82335.o

hp82335-objs := hp82335_init.o

hp82335.o: $(hp82335-objs)
	$(LD) -r -o hp82335.o $(hp82335-objs)

include $(TOPDIR)/Rules.make
