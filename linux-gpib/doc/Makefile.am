# doc/Makefile.am
# copyright (C) 2003 by Frank Mori Hess
# email : fmhess@users.sourceforge.net
#
#   This Makefile.am is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

docs = linux-gpib.pdf #man

SGML = linux-gpib.sgml gpib_version.txt

EXTRA_DIST = linux-gpib.sgml fdl.xml gpib_version.txt $(doc_DATA) $(docs)

if BUILD_DOCS
all-local: $(docs)
doc_DATA = doc_html/*
else
all-local:
doc_DATA =
endif

docdir = $(pkgdatadir)/html

maintainer-clean-local:
	$(RM) gpib_version.txt
	$(RM) -r $(docs)

linux-gpib.pdf: $(SGML)
	$(JW_PATH) -f docbook -b pdf linux-gpib.sgml

doc_html/*: $(SGML)
	$(RM) -r doc_html
	{ $(JW_PATH) -f docbook -b html -o doc_html linux-gpib.sgml; } || { $(RM) -r doc_html; exit 1; }

man/*: $(SGML)
	$(RM) -r man
	{ $(JW_PATH) -f docbook -b man -o man linux-gpib.sgml; } || { $(RM) -r man; exit 1; }

gpib_version.txt: $(top_srcdir)/configure.ac
	echo $(VERSION) > gpib_version.txt

