#!/bin/sh
cd $1
echo "/* Automatically created by mkproto.sh. DONT EDIT */" 
echo "/***** Public Functions ******/" 
grep -h IBLCL *.c|/lib/cpp -P -DIBLCL=extern -DVOID=void| sed 's/)/);/' 
