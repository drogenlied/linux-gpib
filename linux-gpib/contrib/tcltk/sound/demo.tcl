#!/usr/local/bin/wish 

load ./libsound.so 

set warnfont -adobe-helvetica-bold-r-*-*-60-*-*-*-*-*-*-*

set color 0
set expired 6
######################################################################
proc blink { } {
global color expired
if { $color } {
.f0.l0 configure -background papayawhip
.f0.l2 configure -background red
set color 0
} else {
set color 1
.f0.l0 configure -background red
.f0.l2 configure -background papayawhip
}

after 500 blink

}

######################################################################
proc alert { }   {
global expired
sound play 1 1
if { !$expired } {
  sound play 3 3 
  quit
} else {
set expired [expr $expired - 1 ]
after 3000 alert
 if { ![expr $expired % 2] } {
		sound play 2 2
 }
}
}

######################################################################
proc quit { } {
#sound play 3 1
#sleep 5
sound restore; destroy .
}

frame  .f0 -relief raised -borderwidth 10
label  .f0.l0 -text "Self Destruction" -font $warnfont
label  .l1 -text "Quick and Dirty Sound Demo"
label  .f0.l2 -textvariable expired -font $warnfont
button .c -text "Clink" -command { sound play 0 0 }
button .k -text "Press Me " -command {
        pack forget .q .k .l1 .c
        pack .f0
        pack .f0.l0  -fill x -expand yes -padx 20 -pady 20
        pack .f0.l2 -fill x -expand yes -padx 20 -pady 20
	alert ;blink}

button .q -text "quit"  -command { quit }

pack .l1 -fill x
pack .c -fill x 
pack .k -fill x
pack .q -fill x


sound load "clink.raw"
#sound load "alarm2.au"
sound load "buzzer.au"
sound load "gefahr.au"
sound load "crash.au"

#sound load "klaxxon.au"


sound init



