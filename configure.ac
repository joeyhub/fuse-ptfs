#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.69)
AC_INIT(fuse-ptfs, 1.0, https://github.com/joeyhub/fuse-ptfs)
AM_INIT_AUTOMAKE(fuse-ptfs, 1.0)
AC_CONFIG_SRCDIR([src/fuse_ptfs.c])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h stdlib.h string.h unistd.h])

PKG_CHECK_MODULES(FUSE, [fuse >= 2.8])
PKG_CHECK_MODULES(LIBPARTED, [libparted >= 2.1])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset strerror])

AC_OUTPUT(Makefile src/Makefile)
