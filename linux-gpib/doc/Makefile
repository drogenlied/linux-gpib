#    doc/Makefile
#
#    copyright            : (C) 2002 by Frank Mori Hess
#    email                : fmhess@users.sourceforge.net
#
########################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
########################################################################

all: html man pdf

gpib_version.txt:
	cp ../util/version gpib_version.txt

html: linux-gpib.sgml gpib_version.txt
	docbook2html -o html linux-gpib.sgml && touch html

man: linux-gpib.sgml gpib_version.txt
	docbook2man -o man linux-gpib.sgml && touch man

pdf: linux-gpib.sgml gpib_version.txt
	docbook2pdf -o pdf linux-gpib.sgml && touch pdf

clean:
	rm -rf html
	rm -rf man
	rm -rf pdf
	rm -f gpib_version.txt

