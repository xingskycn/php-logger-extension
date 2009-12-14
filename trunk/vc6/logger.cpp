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
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id: header,v 1.16.2.1.2.1 2007/01/01 19:32:09 iliaa Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include "zend_execute.h"
#include "php.h"
#include "php_globals.h"
#include "ext/standard/info.h"
#include "php_logger.h"
#include "php_ini.h"

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/helpers/exception.h>

using namespace log4cxx;

ZEND_DECLARE_MODULE_GLOBALS(logger)
static PHP_GINIT_FUNCTION(logger);

/* NTS globals var */
static zend_class_entry *logger_basic_configurator_ce;
static zend_object_handlers logger_basic_configurator_object_handlers;

static zend_class_entry *logger_property_configurator_ce;
static zend_object_handlers logger_property_configurator_object_handlers;

static zend_class_entry *logger_manager_ce;
static zend_object_handlers logger_manager_object_handlers;

static zend_class_entry *logger_ce;
static zend_object_handlers logger_object_handlers;
struct logger_object {
	zend_object std;
	LoggerPtr logger;
};


static int le_logger;

/* {{{ logger_functions[]
 *
 * User functions
 */
zend_function_entry logger_functions[] = {
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ logger_module_entry
 */
zend_module_entry logger_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	PHP_LOGGER_EXTNAME,
	logger_functions,
	PHP_MINIT(logger),
	PHP_MSHUTDOWN(logger),
	PHP_RINIT(logger),
	PHP_RSHUTDOWN(logger),
	PHP_MINFO(logger),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_LOGGER_VERSION,
#endif
	PHP_MODULE_GLOBALS(logger),
	PHP_GINIT(logger),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_LOGGER
ZEND_GET_MODULE(logger)
#endif


/* {{{ proto public static void configure()
   Add a ConsoleAppender that uses Pattern Layout and prints to stdout to the root logger. */
static PHP_METHOD(LoggerBasicConfigurator, configure)
{
	BasicConfigurator::configure();
}
/* }}} */

function_entry logger_basic_configurator_methods[] = {
	PHP_ME(LoggerBasicConfigurator, configure, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

/* {{{ proto public static boolean configure(string $file)
   Read configuration options from $file. */
static PHP_METHOD(LoggerPropertyConfigurator, configure)
{
	char *file;
	int file_len;
	
	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &file, &file_len)) {
		RETURN_FALSE;
	}
	
	PropertyConfigurator::configure(file);
	RETURN_TRUE;
}
/* }}} */

function_entry logger_property_configurator_methods[] = {
	PHP_ME(LoggerPropertyConfigurator, configure, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

/* {{{ proto public static Logger getLogger(string $name)
   Returns the specific logger. */
static PHP_METHOD(LoggerManager, getLogger)
{
	char *name;
	int name_len;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len)) {
		RETURN_FALSE;
	}

	/* create an instance of Logger */
	object_init_ex(return_value, logger_ce);
	logger_object *obj = (logger_object *)zend_object_store_get_object(return_value TSRMLS_CC);
	obj->logger = LogManager::getLogger(name);
}
/* }}} */

function_entry logger_manager_methods[] = {
	PHP_ME(LoggerManager, getLogger, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

void logger_free_storage(void *object TSRMLS_DC)
{
	logger_object *obj = (logger_object *)object;
	// delete obj->logger; not needed because obj->logger is statically handled by log4cxx
	
	zend_hash_destroy(obj->std.properties);
	FREE_HASHTABLE(obj->std.properties);
	
	efree(obj);
}

zend_object_value logger_create_handler(zend_class_entry *type TSRMLS_DC)
{
	zval *tmp;
	zend_object_value retval;
	
	logger_object *obj = (logger_object *)emalloc(sizeof(logger_object));
	memset(obj, 0, sizeof(logger_object));
	obj->std.ce = type;
	
	ALLOC_HASHTABLE(obj->std.properties);
	zend_hash_init(obj->std.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
	zend_hash_copy(obj->std.properties, &type->default_properties, 
			(copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));
	
	retval.handle = zend_objects_store_put(obj, NULL,
			logger_free_storage, NULL TSRMLS_CC);
	retval.handlers = &logger_object_handlers;
	
	return retval;
}

char *php_logger_decode_level_int(int level) { /* {{{ */
	switch (level) {
		case Level::FATAL_INT:
			return "FATAL";
		break;
		case Level::ERROR_INT:
			return "ERROR";
		break;
		case Level::WARN_INT:
			return "WARN";
		break;
		case Level::INFO_INT:
			return "INFO";
		break;
		case Level::DEBUG_INT:
			return "DEBUG";
		break;
		case Level::TRACE_INT:
			return "TRACE";
		break;
		default:
			return NULL;
		break;
	}
}
/* }}} */

/* Compute Class::method or Function name */
void php_logger_get_executing_method_name(char **origin TSRMLS_DC) { /* {{{ */
	char *space = "";
	char *class_name = NULL;
	char *function = NULL;
	int origin_len;

	function = get_active_function_name(TSRMLS_C);
	class_name = get_active_class_name(&space TSRMLS_CC);

	if (space[0] == '\0' && !function) {
		*origin = (char *)emalloc(5);
		memcpy(*origin, "main", 5);
		return;
	}

	origin_len = spprintf(origin, 0, "%v%s%v", class_name, space, function);
	if (!origin_len) {
		*origin = (char *)emalloc(8);
		memcpy(*origin, "unknown", 8);
		return;
	}
}
/* }}} */


/* {{{ proto public void getLogger(string $message)
   Log a message with the DEBUG level. */
static PHP_METHOD(Logger, debug)
{
	char *message;
	int message_len;
	zval *object = getThis();

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &message, &message_len)) {
		return;
	}

	logger_object *obj = (logger_object *)zend_object_store_get_object(object TSRMLS_CC);
	LoggerPtr logger = obj->logger;

	if (LOG4CXX_UNLIKELY(logger->isDebugEnabled())) {
		helpers::MessageBuffer oss_;
		char *location;
		php_logger_get_executing_method_name(&location TSRMLS_CC);

		logger->forcedLog(
			::log4cxx::Level::getDebug(),
			oss_.str(oss_ << message),
			spi::LocationInfo(
				zend_get_executed_filename(TSRMLS_C),
				location,
				zend_get_executed_lineno(TSRMLS_C))
		);
		efree(location);
	}
}
/* }}} */

/* {{{ proto public void getLogger(string $message)
   Log a message with the TRACE level. */
static PHP_METHOD(Logger, trace)
{
	char *message;
	int message_len;
	zval *object = getThis();

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &message, &message_len)) {
		return;
	}

	logger_object *obj = (logger_object *)zend_object_store_get_object(object TSRMLS_CC);
	LoggerPtr logger = obj->logger;

	if (LOG4CXX_UNLIKELY(logger->isTraceEnabled())) {
		helpers::MessageBuffer oss_;
		char *location;
		php_logger_get_executing_method_name(&location TSRMLS_CC);

		logger->forcedLog(
			::log4cxx::Level::getTrace(),
			oss_.str(oss_ << message),
			spi::LocationInfo(
				zend_get_executed_filename(TSRMLS_C),
				location,
				zend_get_executed_lineno(TSRMLS_C))
		);
		efree(location);
	}
}
/* }}} */

/* {{{ proto public void getLogger(string $message)
   Log a message with the INFO level. */
static PHP_METHOD(Logger, info)
{
	char *message;
	int message_len;
	zval *object = getThis();

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &message, &message_len)) {
		return;
	}

	logger_object *obj = (logger_object *)zend_object_store_get_object(object TSRMLS_CC);
	LoggerPtr logger = obj->logger;
	
	if (LOG4CXX_UNLIKELY(logger->isInfoEnabled())) {
		helpers::MessageBuffer oss_;
		char *location;
		php_logger_get_executing_method_name(&location TSRMLS_CC);

		logger->forcedLog(
			::log4cxx::Level::getInfo(),
			oss_.str(oss_ << message),
			spi::LocationInfo(
				zend_get_executed_filename(TSRMLS_C),
				location,
				zend_get_executed_lineno(TSRMLS_C))
		);
		efree(location);
	}
}
/* }}} */

/* {{{ proto public void getLogger(string $message)
   Log a message with the WARN level. */
static PHP_METHOD(Logger, warn)
{
	char *message;
	int message_len;
	zval *object = getThis();

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &message, &message_len)) {
		return;
	}

	logger_object *obj = (logger_object *)zend_object_store_get_object(object TSRMLS_CC);
	LoggerPtr logger = obj->logger;
	
	if (LOG4CXX_UNLIKELY(logger->isWarnEnabled())) {
		helpers::MessageBuffer oss_;
		char *location;
		php_logger_get_executing_method_name(&location TSRMLS_CC);

		logger->forcedLog(
			::log4cxx::Level::getWarn(),
			oss_.str(oss_ << message),
			spi::LocationInfo(
				zend_get_executed_filename(TSRMLS_C),
				location,
				zend_get_executed_lineno(TSRMLS_C))
		);
		efree(location);
	}
}
/* }}} */

/* {{{ proto public void getLogger(string $message)
   Log a message with the ERROR level. */
static PHP_METHOD(Logger, error)
{
	char *message;
	int message_len;
	zval *object = getThis();

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &message, &message_len)) {
		return;
	}

	logger_object *obj = (logger_object *)zend_object_store_get_object(object TSRMLS_CC);
	LoggerPtr logger = obj->logger;

	if (LOG4CXX_UNLIKELY(logger->isErrorEnabled())) {
		helpers::MessageBuffer oss_;
		char *location;
		php_logger_get_executing_method_name(&location TSRMLS_CC);

		logger->forcedLog(
			::log4cxx::Level::getError(),
			oss_.str(oss_ << message),
			spi::LocationInfo(
				zend_get_executed_filename(TSRMLS_C),
				location,
				zend_get_executed_lineno(TSRMLS_C))
		);
		efree(location);
	}
}
/* }}} */

/* {{{ proto public void getLogger(string $message)
   Log a message with the FATAL level. */
static PHP_METHOD(Logger, fatal)
{
	char *message;
	int message_len;
	zval *object = getThis();

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &message, &message_len)) {
		return;
	}

	logger_object *obj = (logger_object *)zend_object_store_get_object(object TSRMLS_CC);
	LoggerPtr logger = obj->logger;

	if (LOG4CXX_UNLIKELY(logger->isFatalEnabled())) {
		helpers::MessageBuffer oss_;
		char *location;
		php_logger_get_executing_method_name(&location TSRMLS_CC);

		logger->forcedLog(
			::log4cxx::Level::getFatal(),
			oss_.str(oss_ << message),
			spi::LocationInfo(
				zend_get_executed_filename(TSRMLS_C),
				location,
				zend_get_executed_lineno(TSRMLS_C))
		);
		efree(location);
	}
}
/* }}} */

/* {{{ proto public string getLevel()
   Returns the assigned Level, if any, for this Logger. Return null if no level assigned to logger */
static PHP_METHOD(Logger, getLevel) {
	zval *object = getThis();
	logger_object *obj = (logger_object *)zend_object_store_get_object(object TSRMLS_CC);
	LevelPtr level;
	char *levelstr;
	
	level = obj->logger->getLevel();
	if (!level) {
		RETURN_NULL();
	}

	levelstr = php_logger_decode_level_int(level->toInt());
	if (levelstr) {
		RETURN_STRING(levelstr, 1);
	} else {
		RETURN_NULL();
	}
}
/* }}} */

/* {{{ proto public string getEffectiveLevel()
   Starting from this logger, search the logger hierarchy for a non-null level and return it. */
static PHP_METHOD(Logger, getEffectiveLevel) {
	zval *object = getThis();
	logger_object *obj = (logger_object *)zend_object_store_get_object(object TSRMLS_CC);
	LevelPtr level;
	char *levelstr;
	
	try {
		level = obj->logger->getEffectiveLevel();
	} catch (helpers::NullPointerException&) {
		RETURN_NULL();
	}

	if (!level) {
		RETURN_NULL();
	}

	levelstr = php_logger_decode_level_int(level->toInt());
	if (levelstr) {
		RETURN_STRING(levelstr, 1);
	} else {
		RETURN_NULL();
	}
}
/* }}} */

function_entry logger_methods[] = {
	PHP_ME(Logger, debug, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Logger, trace, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Logger, info , NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Logger, warn , NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Logger, error, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Logger, fatal, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Logger, getLevel, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Logger, getEffectiveLevel, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("logger.configuration", "log4php.properties", PHP_INI_ALL, OnUpdateString, configuration, zend_logger_globals, logger_globals)
PHP_INI_END()
/* }}} */

static PHP_GINIT_FUNCTION(logger)
{
	// LOGGER_G(configuration) = "";
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(logger)
{
	zend_class_entry ce;

	REGISTER_INI_ENTRIES();

	/* initialize class LoggerBasicConfigurator */ 
	INIT_CLASS_ENTRY(ce, "LoggerBasicConfigurator", logger_basic_configurator_methods);
	logger_basic_configurator_ce = zend_register_internal_class(&ce TSRMLS_CC);
	memcpy(&logger_basic_configurator_object_handlers,
	        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	logger_basic_configurator_object_handlers.clone_obj = NULL;

	/* initialize class LoggerPropertyConfigurator */
	INIT_CLASS_ENTRY(ce, "LoggerPropertyConfigurator", logger_property_configurator_methods);
	logger_property_configurator_ce = zend_register_internal_class(&ce TSRMLS_CC);
	memcpy(&logger_property_configurator_object_handlers,
	        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	logger_property_configurator_object_handlers.clone_obj = NULL;

	/* initialize class LoggerManager */
	INIT_CLASS_ENTRY(ce, "LoggerManager", logger_manager_methods);
	logger_manager_ce = zend_register_internal_class(&ce TSRMLS_CC);
	memcpy(&logger_manager_object_handlers,
	        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	logger_manager_object_handlers.clone_obj = NULL;

	/* initialize class Logger */
	INIT_CLASS_ENTRY(ce, "Logger", logger_methods);
	logger_ce = zend_register_internal_class(&ce TSRMLS_CC);
	logger_ce->create_object = logger_create_handler;
	memcpy(&logger_object_handlers,
	        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	logger_object_handlers.clone_obj = NULL;

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(logger)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(logger)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(logger)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(logger)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "logger support", "enabled");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
