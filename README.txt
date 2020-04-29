The Linux GPIB Package
 -----------------------------------
  (c) 1994-1996 by C.Schroeter
  (c) 2001-2011 by Frank Mori Hess

=====================================================

This is a GPIB/IEEE-488 driver and utility package for LINUX.

This software distribution package linux-gpib-x.x.x.tar.gz contains
this README and two tarballs:

1) kernel modules in linux-gpib-kernel-x.x.x.tar.gz

2) user space software in linux-gpib-use-x.x.x.tar.gz containing the
   config program, library, device scripts, examples and documentation

Untar each file and see the respective INSTALL files for instructions
on building and installing.

Send comments, questions and suggestions to to the linux-gpib mailing
list at linux-gpib-general@lists.sourceforge.net

Release Notes for linux-gpib-4.3.1
----------------------------------

Changes since the linux-gpib-4.3.0 release

	- Update doc for certain NI GPIB-USB-HS+ that may need a one
          time firmware download

	- Make the NI GPIB-USB-HS+ LED stop blinking green/yellow.

	- Re-enable support for PCMCIA

	- Resubmit interrupt urbs in ni_usb driver based on bug
	report/patch from Matthias Babel

	- Remove support for gpib_config --serial-number option

	- Improve udev rules

	- Updates for newer kernels

	- More details can be found in the Changelog since R1857
	  
Note: If you have any pre 4.3.0 gpib udev rules files in
      /etc/udev/rules.d/ please remove them before installing
      linux-gpib-user-4.3.0.
      
      The files to remove are:
      	   99-agilent_82357a.rules
	   99-gpib-generic.rules
	   99-ni_usb_gpib.rules


