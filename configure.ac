AC_PREREQ(2.63)
AC_INIT(fuse-ptfs, 1.0, https://github.com/joeyhub/fuse-ptfs)
AM_INIT_AUTOMAKE(fuse-ptfs, 1.0)
AC_CONFIG_SRCDIR([src/fuse_ptfs.c])

AC_PROG_CC

AC_CHECK_HEADERS([fcntl.h limits.h stdlib.h string.h unistd.h])

PKG_CHECK_MODULES(FUSE, [fuse >= 2.8])
PKG_CHECK_MODULES(LIBPARTED, [libparted >= 2.1])

AC_TYPE_OFF_T
AC_TYPE_SIZE_T

AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset strerror])

AC_OUTPUT(Makefile src/Makefile)
