# doc/Makefile.am
# copyright (C) 2003 by Frank Mori Hess
# email : fmhess@users.sourceforge.net
#
#   This Makefile.am is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

docs = doc_html linux-gpib.pdf #man

SGML = linux-gpib.sgml gpib_version.txt

EXTRA_DIST = linux-gpib.sgml fdl.xml gpib_version.txt $(docs)

if BUILD_DOCS
all-local: $(docs)
else
all-local:
endif

install-data-local:
#	$(INSTALL) -d $(mandir)/man3
#	$(INSTALL_DATA) man/*.3 $(mandir)/man3
	$(INSTALL) -d $(pkgdatadir)/html
	-$(INSTALL_DATA) doc_html/* $(pkgdatadir)/html

maintainer-clean-local:
	$(RM) gpib_version.txt
	$(RM) -r $(docs)

linux-gpib.pdf: $(SGML)
	$(JW_PATH) -f docbook -b pdf linux-gpib.sgml

doc_html: $(SGML)
	{ $(JW_PATH) -f docbook -b html -o doc_html linux-gpib.sgml && touch doc_html; } || { $(RM) -r doc_html; exit 1; }

man: $(SGML)
	{ $(JW_PATH) -f docbook -b man -o man linux-gpib.sgml && touch man; } || { $(RM) -r man; exit 1; }

gpib_version.txt: $(top_srcdir)/configure.ac
	echo $(VERSION) > gpib_version.txt

