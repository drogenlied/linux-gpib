
include make.config


clean:
	-rm make.config

install:
	for i in *.so* ; do install $$i $(INSTALLDIR) ; done

make.config:
	../config.tcl >make.config