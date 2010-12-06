AC_PREREQ([2.66])
AC_INIT([NetNuke], [2.0.2], [jhunkeler@gmail.com])
AM_INIT_AUTOMAKE(1.11.1) 
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([src/netnuke.c])
AC_CONFIG_HEADERS([config.h])

CFLAGS="-Wall -O2 -I../include"
LDFLAGS="-lpthread"

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99
AS_IF([test "$ac_cv_prog_cc_c99" = "no"],
	[AC_MSG_ERROR([no C99 compiler found. Aborting.])])

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_create], [],
	[AC_MSG_ERROR([required library pthread missing])])

AC_CHECK_LIB([sysfs], [sysfs_read_attribute], [],
	[AC_MSG_ERROR([required library libsysfs missing])])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/ioctl.h unistd.h sysfs/libsysfs.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memmove memset strerror strncasecmp])

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
