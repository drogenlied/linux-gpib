#
#    copyright            : (C) 2003 by Frank Mori Hess
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#

BINS = gpib_config
SRCS = $(BINS:%=%.c)

CPPFLAGS += -I../
LDFLAGS = -L../ -lgpib -lpthread

all: $(BINS) .depend

$(BINS) : % : %.o
	$(CC) -o $@ $< $(LDFLAGS)

clean:
	rm -f *.o $(BINS) .depend

.depend : $(SRCS)
	$(CPP) $(CPPFLAGS) -MM $(SRCS)  >./.depend

install:
	$(INSTALL) -D gpib_config $(TARGET_ROOT)/sbin/gpib_config

-include .depend
