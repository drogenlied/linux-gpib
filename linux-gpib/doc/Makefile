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

html: linux-gpib.sgml
	mkdir -p html
	touch html
	docbook2html -o html linux-gpib.sgml

man: linux-gpib.sgml
	mkdir -p man
	touch man
	docbook2man -o man linux-gpib.sgml

pdf: linux-gpib.sgml
	mkdir -p pdf
	touch pdf
	docbook2pdf -o pdf linux-gpib.sgml

clean:
	rm -rf html
	rm -rf man
	rm -rf pdf

