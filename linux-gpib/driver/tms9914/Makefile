
obj-m += tms9914.o

list-multi := tms9914.o

tms9914-objs := cmd.o init.o read.o util.o write.o aux.o interrupt.o

export-objs := $(tms9914-objs)

tms9914.o: $(tms9914-objs)
	$(LD) -r -o tms9914.o $(tms9914-objs)

