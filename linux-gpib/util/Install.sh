#!/bin/sh
#
# Install.sh: 	Just a wrapper around "install" which prints out the
#		filenames in a nice way.
# Bugs:		Commandline switches must be placed at the end of the line.
#		Can't handle the "create-directory-form". Use mkdir instead.
#

BOLD="\033[1m"
NBOLD="\033[0m"
ECHO=/bin/echo

if [ $# -lt 2 ]; then
	${ECHO} Usage: $0 source-filename destination
	exit 1
fi

${ECHO} --- Installing ${BOLD}$1${NBOLD} to ${BOLD}$2${NBOLD}...
exec install $*

unset BOLD NBOLD ECHO
