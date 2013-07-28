/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2013 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Jarismar C. Silva <jarismar@gmail.com>                      |
   |          Diego Tremper <diegotremper@gmail.com>                      |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef _WIN32
	#include <iostream>
#endif

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
#include <log4cxx/helpers/systemerrwriter.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

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

static zend_class_entry *logger_mdc_ce;
static zend_object_handlers logger_mdc_object_handlers;

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
	STANDARD_MODULE_HEADER_EX,
	NULL,
	NULL,
	PHP_LOGGER_EXTNAME,
	logger_functions,
	PHP_MINIT(logger),
	PHP_MSHUTDOWN(logger),
	PHP_RINIT(logger),
	PHP_RSHUTDOWN(logger),
	PHP_MINFO(logger),
	PHP_LOGGER_VERSION,
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

bool php_logger_load_properties(char *file) { /* {{{ */
	PropertyConfigurator::configure(file);

	if (SystemErrWriter::hasMessage()) {
		return false;
	}

	return true;
}
/* }}} */

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO(arginfo_logger__void, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ proto public static void configure()
   Add a ConsoleAppender that uses Pattern Layout and prints to stdout to the root logger. */
static PHP_METHOD(LoggerBasicConfigurator, configure)
{
	BasicConfigurator::configure();
}
/* }}} */

const zend_function_entry logger_basic_configurator_methods[] = {
	PHP_ME(LoggerBasicConfigurator, configure, arginfo_logger__void, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_loggerpropertyconfigurator_configure, 0, 0, 1)
	ZEND_ARG_INFO(0, file)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ proto public static boolean configure(string $file)
   Read configuration options from $file. */
static PHP_METHOD(LoggerPropertyConfigurator, configure)
{
	char *file;
	int file_len;
	
	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &file, &file_len)) {
		RETURN_FALSE;
	}

	if(php_logger_load_properties(file)) {
		RETURN_TRUE;
	}

	php_error_docref(NULL TSRMLS_CC, E_WARNING, SystemErrWriter::getMessage());
	RETURN_FALSE;
}
/* }}} */

const zend_function_entry logger_property_configurator_methods[] = {
	PHP_ME(LoggerPropertyConfigurator, configure, arginfo_loggerpropertyconfigurator_configure, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_loggermanager_getlogger, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()
/* }}} */


/* {{{ proto public static Logger getLogger(string $name)
   Returns the specific logger. */
static PHP_METHOD(LoggerManager, getLogger)
{
	/* Note: any modification you do here, should also be applied to Logger::getLogger
	 *       I'll look for a way to reference the same method and remove these two copies */

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

const zend_function_entry logger_manager_methods[] = {
	PHP_ME(LoggerManager, getLogger, arginfo_loggermanager_getlogger, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

void logger_free_storage(void *object TSRMLS_DC)
{
	logger_object *obj = (logger_object *)object;
	zend_object_std_dtor(&obj->std TSRMLS_CC);
	efree(obj);
}

zend_object_value logger_logger_new(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value retval;

	logger_object *obj = (logger_object *)emalloc(sizeof(logger_object));
	memset(obj, 0, sizeof(logger_object));
	zend_object_std_init(&obj->std, ce TSRMLS_CC);
	object_properties_init(&obj->std, ce);

	retval.handle = zend_objects_store_put(
		obj,
		NULL,
		logger_free_storage,
		NULL TSRMLS_CC
	);
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

/* Compute namespace::Class::method, Class::method or Function name */
void php_logger_get_executing_method_name(char **origin TSRMLS_DC) { /* {{{ */
	const char *space = "";
	const char *class_name = NULL;
	const char *function = NULL;
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

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_logger_trace, 0, 0, 1)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_logger_debug, 0, 0, 1)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_logger_info, 0, 0, 1)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_logger_warn, 0, 0, 1)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_logger_error, 0, 0, 1)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_logger_fatal, 0, 0, 1)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_logger_getlogger, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()
/* }}} */


/* {{{ proto public void getLogger(string $message)
   Log a message with the DEBUG level. */
static PHP_METHOD(Logger, debug)
{
	char *message;
	int message_len;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &message, &message_len)) {
		return;
	}

	zval *object = getThis();
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

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &message, &message_len)) {
		return;
	}

	zval *object = getThis();
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

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &message, &message_len)) {
		return;
	}

	zval *object = getThis();
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

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &message, &message_len)) {
		return;
	}

	zval *object = getThis();
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

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &message, &message_len)) {
		return;
	}

	zval *object = getThis();
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

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &message, &message_len)) {
		return;
	}

	zval *object = getThis();
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
   Returns the assigned Level, if any, for this Logger. Return null if no level was assigned to this logger */
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

/* {{{ proto public static Logger getLogger(string $message)
   Alias to LoggerManager::getLogger. */
static PHP_METHOD(Logger, getLogger)
{
	/* FIXME: this method is an alias for LoggerManager::getLogger, verify how to
	 * point to it and remove this copy-pasted version */
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

const zend_function_entry logger_methods[] = {
	PHP_ME(Logger, debug, arginfo_logger_debug, ZEND_ACC_PUBLIC)
	PHP_ME(Logger, trace, arginfo_logger_trace, ZEND_ACC_PUBLIC)
	PHP_ME(Logger, info , arginfo_logger_info, ZEND_ACC_PUBLIC)
	PHP_ME(Logger, warn , arginfo_logger_warn, ZEND_ACC_PUBLIC)
	PHP_ME(Logger, error, arginfo_logger_error, ZEND_ACC_PUBLIC)
	PHP_ME(Logger, fatal, arginfo_logger_fatal, ZEND_ACC_PUBLIC)
	PHP_ME(Logger, getLevel, arginfo_logger__void, ZEND_ACC_PUBLIC)
	PHP_ME(Logger, getLogger, arginfo_logger_getlogger, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(Logger, getEffectiveLevel, arginfo_logger__void, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};


/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_loggermdc_put, 0, 0, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_loggermdc_get, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_loggermdc_remove, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ proto public static void put(string $key, string $value)
   Put a context value as identified with the key parameter into the current context map. */
static PHP_METHOD(LoggerMDC, put) {
	char *key, *value;
	int key_len, value_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &key, &key_len, &value, &value_len) == FAILURE)	{
		return;
	}

	MDC::remove(key); // need to call remove because MDC does not support updating the value

	if (SystemErrWriter::hasMessage()) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, SystemErrWriter::getMessage());
		return;
	}

	MDC::put(key, value);
}
/* }}} */

/* {{{ proto public static string remove(string $key)
   Remove the context identified by the key parameter. */
static PHP_METHOD(LoggerMDC, remove) {
	char *key;
	int key_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE)	{
		return;
	}

	std::string value = MDC::remove(key);

	if (SystemErrWriter::hasMessage()) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, SystemErrWriter::getMessage());
		RETURN_FALSE;
	}

	RETURN_STRING(value.c_str(), 1);
}
/* }}} */

/* {{{ proto public static string get(string $key)
   Get the context identified by the key parameter. */
static PHP_METHOD(LoggerMDC, get) {
	char *key;
	int key_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE)	{
		return;
	}

	std::string value = MDC::get(key);

	if (SystemErrWriter::hasMessage()) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, SystemErrWriter::getMessage());
		RETURN_FALSE;
	}

	RETURN_STRING(value.c_str(), 1);
}

const zend_function_entry logger_mdc_methods[] = {
	PHP_ME(LoggerMDC, put, arginfo_loggermdc_put, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(LoggerMDC, get, arginfo_loggermdc_get, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(LoggerMDC, remove, arginfo_loggermdc_remove, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("logger.properties", "", PHP_INI_ALL, OnUpdateString, logger_ini_properties, zend_logger_globals, logger_globals)
PHP_INI_END()
/* }}} */

static PHP_GINIT_FUNCTION(logger)
{
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
	logger_ce->create_object = logger_logger_new;
	memcpy(&logger_object_handlers,
	        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	logger_object_handlers.clone_obj = NULL;

	/* initialize class LoggerMDC */
	INIT_CLASS_ENTRY(ce, "LoggerMDC", logger_mdc_methods);
	logger_mdc_ce = zend_register_internal_class(&ce TSRMLS_CC);
	memcpy(&logger_mdc_object_handlers,
	        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	logger_mdc_object_handlers.clone_obj = NULL;

	/* verify if logger.properties was set on ini file, then try to load the file */
	char *file = LOGGER_G(logger_ini_properties);
	if (strlen(file) > 0) {
		bool ok = php_logger_load_properties(file);
		if (!ok) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, SystemErrWriter::getMessage());
		}
	}

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