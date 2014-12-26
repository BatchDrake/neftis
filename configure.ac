#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(neftis, 0.0.0)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADER([microkernel/include/config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE

cflags_were_set=false
if test "${CFLAGS+set}" = set; then
  AM_CFLAGS=$CFLAGS
  CFLAGS=
  cflags_were_set=:
fi

ccasflags_were_set=false
if test "${CCASFLAGS+set}" = set; then
  AM_CCASFLAGS=$CCASFLAGS
  CCASFLAGS=
  ccasflags_were_set=:
fi

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AM_PROG_AS
AC_PROG_LIBTOOL

# the following needs to come after the expansion of AC_PROG_CC
if $cflags_were_set; then :; else
  AM_CFLAGS=$CFLAGS
  CFLAGS=
fi

if $ccasflags_were_set; then :; else
  AM_CCASFLAGS=$CCASFLAGS
  CCASFLAGS=
fi

AC_SUBST(AM_CCASFLAGS)
AC_SUBST(AM_CFLAGS)

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE

if test "$ac_cv_c_inline" != no ; then
  AC_DEFINE([HAVE_INLINE], [0], [This compiler supports inline functions])
  AC_DEFINE(HAVE_INLINE,1)
  AC_SUBST(HAVE_INLINE)
fi
     
# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 microkernel/loaders/elf/Makefile
                 microkernel/scheds/default/Makefile
                 microkernel/arch/x86/Makefile
                 microkernel/Makefile
		 osabi/elf32-linux-gnu/Makefile
])
AC_OUTPUT

