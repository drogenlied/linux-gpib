# doc/Makefile.am
# copyright (C) 2003 by Frank Mori Hess
# email : fmhess@users.sourceforge.net
#
#   This Makefile.am is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

SGML = linux-gpib.sgml gpib_version.txt
EXTRA_DIST = linux-gpib.sgml fdl.xml

if HAVE_DOCBOOK
all-local: html linux-gpib.pdf #man

install-data-local:
#	$(INSTALL) -d $(mandir)/man3
#	$(INSTALL_DATA) man/*.3 $(mandir)/man3
	$(INSTALL) -d $(pkgdatadir)/html
	$(INSTALL_DATA) html/* $(pkgdatadir)/html
else
endif

clean-local:
	rm -f linux-gpib.pdf gpib_version.txt
	rm -rf html man

linux-gpib.pdf: $(SGML)
	$(JW_PATH) -f docbook -b pdf linux-gpib.sgml

html: $(SGML)
	$(JW_PATH) -f docbook -b html -o html linux-gpib.sgml && touch html

man: $(SGML)
	$(JW_PATH) -f docbook -b man -o man linux-gpib.sgml && touch man

gpib_version.txt:
	echo $(VERSION) > gpib_version.txt
