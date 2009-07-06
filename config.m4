dnl $Id$
dnl config.m4 for logger extension
dnl vim:se ts=4 sw=2 et:

PHP_ARG_WITH(logger, Logger support,
[  --with-logger[=DIR]       DIR is log4cxx install dir (version >= 0.10.0 required).
                          Defaults to [/usr/local]])

if test "$PHP_LOGGER" != "no"; then
	PHP_REQUIRE_CXX

	dnl ===== define default log4cxx install dir =====
	if test "$PHP_LOGGER" = "yes"; then
		LOGGER_DIR="/usr/local"
	fi

	dnl ===== check libs =====
	AC_MSG_CHECKING([log4cxx library])

	LOG4CXXLIBDIR="lib"
	LOG4CXXLIBNAME="liblog4cxx"

	if test -f "$LOGGER_DIR/$LOG4CXXLIBDIR/$LOG4CXXLIBNAME.la" && test -f "$LOGGER_DIR/$LOG4CXXLIBDIR/$LOG4CXXLIBNAME.so"; then
		LOG4CXX_LIB_DIR="$LOGGER_DIR/$LOG4CXXLIBDIR/"
	else
		AC_MSG_ERROR('log4cxx libraries not found in $LOGGER_DIR/$LOG4CXXLIBDIR/')
	fi
	AC_MSG_RESULT($LOG4CXX_LIB_DIR)

	dnl ===== check headers =====
	AC_MSG_CHECKING([log4cxx header files])

	INC_DIR="$LOGGER_DIR/include"
	INC_FILES="$INC_DIR/log4cxx/logger.h $INC_DIR/log4cxx/logmanager.h"

	for i in $INC_FILES; do
		if test ! -f $i; then
			AC_MSG_ERROR('log4cxx headers not found in $INC_DIR')
		fi
	done
	LOG4CXX_INC_DIR="$INC_DIR"
	AC_MSG_RESULT($LOG4CXX_INC_DIR)

	dnl ===== add results to environment =====
	dnl PHP_ADD_LIBRARY_WITH_PATH(log4cxx, $LOG4CXX_LIB_DIR, LOGGER_SHARED_LIBADD)
	dnl PHP_CHECK_LIBRARY(
	dnl  log4cxx,
	dnl  _ZN7log4cxx17BasicConfigurator9configureEv,
	dnl  [],
	dnl  [AC_MSG_ERROR(Wrong log4cxx lib version or lib not found)]
	dnl  [-L$LOG4CXX_LIB_DIR $LOGGER_SHARED_LIBADD]
	dnl )

	dnl PHP_ADD_LIBPATH($LOG4CXX_LIB_DIR, LOGGER_SHARED_LIBADD)
	PHP_ADD_LIBRARY_WITH_PATH(log4cxx, "", LOGGER_SHARED_LIBADD)
	dnl PHP_ADD_LIBRARY(log4cxx, 1, LOGGER_SHARED_LIBADD)

  	PHP_ADD_INCLUDE($LOG4CXX_INC_DIR)

	AC_DEFINE([HAVE_LOGGER],1 ,[whether you have logger module])
	PHP_NEW_EXTENSION(logger, logger.cc, $ext_shared,,,yes)
fi