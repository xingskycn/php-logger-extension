dnl $Id$
dnl config.m4 for logger extension
dnl vim:se ts=4 sw=2 et:

PHP_ARG_WITH(logger, for Apache log4cxx support,
[  --with-logger[=DIR]       DIR is log4cxx install dir (version >= 0.10.0 required)
                          Defaults to [/usr/local]])

if test "$PHP_LOGGER" != "no"; then
  PHP_REQUIRE_CXX

  dnl ===== define default log4cxx install dir =====
  LOGGER_BASE_DIR="$PHP_LOGGER"
  if test "$PHP_LOGGER" = "yes"; then
    LOGGER_BASE_DIR="/usr/local"
  fi

  dnl ===== check libs =====
  AC_MSG_CHECKING([log4cxx library])


  if test -f "$LOGGER_BASE_DIR/lib/liblog4cxx.la" &&
     test -f "$LOGGER_BASE_DIR/lib/liblog4cxx.so"; then
    LOG4CXX_LIB_DIR="$LOGGER_BASE_DIR/lib/"
  else
    AC_MSG_ERROR('log4cxx libraries not found in $LOGGER_BASE_DIR/lib/')
  fi
  AC_MSG_RESULT($LOG4CXX_LIB_DIR)

  dnl ===== check headers =====
  AC_MSG_CHECKING([log4cxx header files])

  LOG4CXX_INC_DIR="$LOGGER_BASE_DIR/include/log4cxx"
  INC_FILES="$LOG4CXX_INC_DIR/logger.h $LOG4CXX_INC_DIR/logmanager.h"

  for i in $INC_FILES; do
    if test ! -f $i; then
      AC_MSG_ERROR('log4cxx headers not found in $LOGGER_INC_DIR')
    fi
  done

  AC_MSG_RESULT($LOG4CXX_INC_DIR)

  dnl ===== add log4cxx lib to compile environment =====
  PHP_CHECK_LIBRARY(
    log4cxx,
    _ZN7log4cxx17BasicConfigurator9configureEv,
    [
      AC_DEFINE(HAVE_LIBLOG4CXX,1,[ ])
    ],
    [
      AC_MSG_ERROR([Invalid liblog4cxx, missing log4cxx::BasicConfigurator::configure method.])
    ],
    [
      -L$LOG4CXX_LIB_DIR -llog4cxx
    ]
  )

  AC_DEFINE([HAVE_LOGGER],1 ,[whether you have logger module])

  PHP_ADD_LIBRARY_WITH_PATH(log4cxx, $LOG4CXX_LIB_DIR, LOGGER_SHARED_LIBADD)
  PHP_ADD_INCLUDE($LOG4CXX_INC_DIR)
  PHP_SUBST(LOGGER_SHARED_LIBADD)
  PHP_NEW_EXTENSION(logger, logger.cpp, $ext_shared,,,yes)
fi
