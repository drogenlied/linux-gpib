#!/usr/bin/tclsh

if { [catch [info commands load] ]  } {

puts "INSTALLDIR = [info library]"


} else {

exit 1	

}
