#!/bin/sh
#
# Remove.sh: 	Just a wrapper around "rm" which prints out the
#		filenames in a nice way.
# Bugs:		Can't handle any extra commandline switches.
#


BOLD="\033[1m"
NBOLD="\033[0m"
ECHO="/bin/echo -n"

echoed=0

for FILE in $*; do
    if [ -f $FILE ]; then
    	if [ $echoed -eq 0 ]; then
    	    $ECHO "--- Removing "${BOLD}
	    echoed=1
    	fi
    	#/bin/echo -n `basename $FILE`" "
    	/bin/echo -n $FILE" "
    	rm -f $FILE
    fi
done

if [ $echoed -eq 1 ]; then
    ${ECHO} ${NBOLD}"done\n"
fi

unset echoed
unset BOLD NBOLD ECHO
