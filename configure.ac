AC_PREREQ([2.66])
AC_INIT([NetNuke], [2.0.2], [jhunkeler@gmail.com], [netnuke], [https://github.com/jhunkeler/NetNuke])
AM_INIT_AUTOMAKE(1.11.1) 

AC_CONFIG_SRCDIR([src/netnuke.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SUBDIRS([sysfsutils])

CFLAGS="-Wall -O2 -I../include -static"
LDFLAGS="-lpthread -lrt"

# Checks for programs.
AC_PROG_CC_C99
AS_IF([test "$ac_cv_prog_cc_c99" = "no"],
	[AC_MSG_ERROR([no C99 compiler found. Aborting.])])

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_create], [],
	[AC_MSG_ERROR([required library pthread missing])])

# Checks for header files.
AC_CHECK_HEADERS_ONCE([fcntl.h stdlib.h string.h sys/ioctl.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS_ONCE([memmove memset strerror strncasecmp])

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
