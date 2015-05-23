## Php Logger ##

This project is a php logging framework that uses the apache's [log4cxx](http://logging.apache.org/log4cxx/index.html) c++ library as a php extension.

The main purpose of this project is to build a replacement for the [log4php](http://logging.apache.org/log4php/) logging framework. When using the log4php, your application will need to parse and load the whole framework on every request, the application needs to pay this cost even when the logger is configured to a low level of verbosity (like FATAL or ERROR) thus affecting the application performance not only on development but also in your production environment.

By using the php\_logger the application just need to load the extension once, this is done when the web server is initialized, also the application does not need to parse any php classes in order to be able to log.

If you have a dedicated environment, then you can configure the logger only once by setting logger.properties on php.ini, which is impossible with log4php, by doing this you will reduce the io needed to read the configurarion file on each request.

### Install ###

**Windows stable zip package**
  1. Download log4cxx.dll and php\_logger.dll compatible with your php version and system;
  1. Place log4cxx.dll into php folder (or any other folder on your system path);
  1. Place php\_logger.dll into your php extension folders;
  1. Edit your php.ini to add extension=php\_logger.dll

  * [log4cxx.dll ](https://googledrive.com/host/0B49cfV9rk6A9TDhReEpuWUU3a3M/log4cxx.dll) (v0.10.0, win7, x86, vc9, ts)
  * For php 5.4 [php\_logger.dll](https://googledrive.com/host/0B49cfV9rk6A9RmItX0hMclFucWc/php_logger.dll) (v1.0, win7, x86, **php 5.4, vc9, ts**)
  * For php 5.3 [php\_logger.dll](https://googledrive.com/host/0B49cfV9rk6A9cWRsOU1MdklTams/php_logger.dll) (v0.5, win7, x86, **php 5.3, vc9, ts**)

**Compiling from sources on Linux:**
> See BuildingFromSource.

### Releases ###

#### Release 1.0 (2013-jul-28) ####
  * Supporting php/os's
    * PHP 5.4 TS - Win7 - vc9 - x86
  * Improvements on error handling logic
  * Added support for configuring only once on Apache start/restart by using the logger.properties from php.ini file

#### Release 0.5 (2013-jul-28) ####
  * Supporting php/os's
    * PHP 5.3 TS - Win7 - vc9 - x86
  * Added new class/methods
    * Logger::getLogger - a shorthand to LoggerManager::getLogger
  * LoggerMDC - a class to inject mapped diagnostic context into logger messages
    * LoggerMDC::put
    * LoggerMDC::get
    * LoggerMDC::remove
  * Support for configuring only once backported from v1.0

#### Release 0.0.0.3 (canceled): ####
  * Fixed [Issue7](https://code.google.com/p/php-logger-extension/issues/detail?id=7) Conversion pattern %F:%L is misleading (Thanks to [pcdinh](http://code.google.com/u/pcdinh)).

#### Release 0.0.0.2 (2009-aug-22): ####
  * Introduced new methods
    * Logger::getLevel
    * Logger::getEffectiveLevel
  * Fixed unit tests on windows

#### Release 0.0.0.1 (2009-aug-08): ####
  * Interface to access the liblog4cxx library classes
  * Convert errors from liblog4cxx to php error handler
  * Create appender for logging on console
  * Create appender for logging on file system