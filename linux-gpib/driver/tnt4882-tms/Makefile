
obj-m += tnt4882_tms.o

list-multi := tnt4882_tms.o

tnt4882_tms-objs := init.o mite.o interrupt.o

tnt4882_tms.o: $(tnt4882_tms-objs)
	$(LD) -r -o tnt4882_tms.o $(tnt4882_tms-objs)

include $(TOPDIR)/Rules.make
