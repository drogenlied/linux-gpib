
This version of Linux-GPIB is no longer distributed with all the old
Tcl/Tk stuff called 'ibsh'. The tcl language extension for the gpib system
now is provided as dynamically loadable tcl-module that can be used with
newer tcl/tk versions (I use tcl7.6 and tk4.2)

If you 'make install' the extension will automatically installed to your
actual tcl library path.

to load this extension just add the line

load [info library]/libgpib-tcl.so

near the beginning of your scripts.
now you may use the gpib tcl commands as usual.


cheers
clausi

