#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([alarm], [1.0], [rlp1938@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([alarm.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h fcntl.h limits.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_OFF_T
AC_TYPE_MODE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_CHECK_FUNCS([memchr strchr strdup strerror memset])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
