/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2007 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Jarismar C. Silva <jarismar.php@gmail.com>                   |
  |          Diego Tremper <diegotremper@gmail.com>                      |
  +----------------------------------------------------------------------+
*/

/* $Id$ */
#ifndef PHP_LOGGER_H
#define PHP_LOGGER_H

#define PHP_LOGGER_VERSION "0.5"
#define PHP_LOGGER_EXTNAME "logger"

BEGIN_EXTERN_C()

extern zend_module_entry logger_module_entry;
#define phpext_logger_ptr &logger_module_entry

#ifdef PHP_WIN32
#define PHP_LOGGER_API __declspec(dllexport)
#else
#define PHP_LOGGER_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(logger);
PHP_MSHUTDOWN_FUNCTION(logger);
PHP_RINIT_FUNCTION(logger);
PHP_RSHUTDOWN_FUNCTION(logger);
PHP_MINFO_FUNCTION(logger);

ZEND_BEGIN_MODULE_GLOBALS(logger)
	char *logger_ini_properties;
ZEND_END_MODULE_GLOBALS(logger)

#ifdef ZTS
#define LOGGER_G(v) TSRMG(logger_globals_id, zend_logger_globals *, v)
#else
#define LOGGER_G(v) (logger_globals.v)
#endif

END_EXTERN_C()

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
