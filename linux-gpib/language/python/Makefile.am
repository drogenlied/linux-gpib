# python/Makefile.am
# copyright (C) 2003 by Frank Mori Hess
# email : fmhess@users.sourceforge.net
#
#   This Makefile.am is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

EXTRA_DIST = gpibtest.py setup.py Gpib.py

all-local: binding

binding:
	$(PYTHON) setup.py build

install-data-local:
	$(PYTHON) setup.py install

clean-local:
	rm -rf build

