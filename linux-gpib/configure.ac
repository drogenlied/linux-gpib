
dnl Process this file with autoconf to produce a configure script.

AC_INIT
dnl Every other copy of the package version number gets its value from here
AM_INIT_AUTOMAKE(linux-gpib, 3.1.94-cvs)

dnl create a config.h file (Automake will add -DHAVE_CONFIG_H)
AM_CONFIG_HEADER(config.h)

AC_SUBST(VERSION)

ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

AC_CANONICAL_HOST

AC_ENABLE_SHARED(yes)
AC_ENABLE_STATIC(no)
AC_ARG_ENABLE([pcmcia],[  --enable-pcmcia	Enable support for PCMCIA cards in GPIB kernel drivers],
	[ENABLE_PCMCIA=$enableval],[ENABLE_PCMCIA="no"])
if test $ENABLE_PCMCIA == yes ; then
	AC_DEFINE([GPIB_CONFIG_PCMCIA],[1],[Define to enable pcmcia support in drivers])
fi
AC_ARG_ENABLE([driver-debug],[  --enable-driver-debug	Enable debug spam to console in GPIB kernel drivers],
	[ENABLE_DRIVER_SPAM=$enableval],[ENABLE_DRIVER_SPAM="no"])
if test $ENABLE_DRIVER_SPAM == yes ; then
	AC_DEFINE([GPIB_CONFIG_KERNEL_DEBUG],[1],[Define to enable debug spam to console in drivers])
fi

dnl check kernel source directory
AC_ARG_WITH(linux-srcdir,
	[  --with-linux-srcdir=DIR	location of Linux kernel source directory [[DIR=/lib/modules/$(uname -r)/build/]]],
	[LINUX_SRCDIR=$withval],[LINUX_SRCDIR=/lib/modules/$(uname -r)/build/])
if test $LINUX_SRCDIR == no ; then
	AC_MSG_ERROR([A Linux kernel source directory is required to compile driver modules.  Use --with-linux-srcdir=DIR.])
fi
AC_SUBST(LINUX_SRCDIR)
[echo -n checking Linux kernel directory... ]
if [[ ! -d "$LINUX_SRCDIR" ]];then
	AC_MSG_ERROR([Linux kernel directory $LINUX_SRCDIR does not exist.  Specify using --with-linux-srcdir=DIR.])
fi
if [[ ! -f "$LINUX_SRCDIR/.config" ]];then
	AC_MSG_ERROR([Kernel source tree at $LINUX_SRCDIR is not configured.  Copy the
	appropriate configuration file to $LINUX_SRCDIR/.config and then run \'make oldconfig\'
	in the kernel source directory.])
fi
if [[ ! -f "$LINUX_SRCDIR/.hdepend" ]];then
	AC_MSG_ERROR([You need to run \'make dep\' on the kernel source before continuing.])
fi
[echo \ ok]
[echo -n checking for Linux kernel compilation flags... ]
dnl get compile flags for modules from linux kernel source tree
(cd util/linux_flags && make --quiet LINUXDIR=$LINUX_SRCDIR)
if [[ $? != 0 ]]; then
	AC_MSG_ERROR([Failed to get compile flags from Linux kernel directory.])
fi
[echo \ ok]

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC
AM_PROG_LEX
AC_PROG_YACC
AC_PROG_LIBTOOL
AC_PATH_PROG(JW_PATH, jw, no)
if test $JW_PATH == no ; then
  AC_MSG_WARN([docbook-tools (jw) not found, disabling documentation])
fi
AM_CONDITIONAL(HAVE_DOCBOOK, [test "$JW_PATH" != no])
AM_PATH_PYTHON

dnl Checks for libraries.

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h sys/param.h sys/time.h time.h sys/mkdev.h sys/sysmacros.h string.h memory.h fcntl.h dirent.h sys/ndir.h ndir.h alloca.h locale.h )

AC_HEADER_MAJOR
AC_FUNC_ALLOCA
AC_STRUCT_TM
AC_STRUCT_ST_BLOCKS
AC_FUNC_CLOSEDIR_VOID
AC_CHECK_FUNCS(mkfifo)
AC_CHECK_FUNC(mknod)

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Checks for library functions.

LIBGPIB_CFLAGS="-I\$(top_srcdir)/include -Wall -D_REENTRANT"
LIBGPIB_LDFLAGS="\$(top_srcdir)/lib/libgpib.la -lpthread"
AC_SUBST(LIBGPIB_CFLAGS)
AC_SUBST(LIBGPIB_LDFLAGS)

AC_OUTPUT([\
	Makefile \
	lib/Makefile \
	lib/gpib_config/Makefile \
	examples/Makefile \
	test/Makefile \
	driver/Makefile \
	driver/include/Makefile \
	driver/cb7210/Makefile \
	driver/cec/Makefile \
	driver/hp82335/Makefile \
	driver/ines/Makefile \
	driver/nec7210/Makefile \
	driver/pc2/Makefile \
	driver/sys/Makefile \
	driver/tms9914/Makefile \
	driver/tnt4882/Makefile \
	driver/tnt4882-tms/Makefile \
	driver//Makefile \
	doc/Makefile \
	include/Makefile \
	language/guile/Makefile \
	language/python/Makefile \
	language/tcl/Makefile \
]
)

