# Introduction #
Here are some instructions to build the extension from the source files. We have support compiling on linux and windows xp.

## Preparation ##
  1. Download php source code from http://www.php.net/downloads.php. At the moment we have sucessfully compiled with php-5.2.10.
  1. Download the latest log4cxx (>= 0.10.0) from http://logging.apache.org/log4cxx/download.html
  1. The logger has the same pre-requisites as php to compile. e.g. you will need an update gcc on linux and the msvc6 on windows.

## Building on Linux ##
  1. Before building log4cxx you will need to download the modified versions of some log4cxx files. Please see what files to donwload [here](https://code.google.com/p/php-logger-extension/source/browse/trunk/log4cxx_modified_files/list_of_modified_files.TXT).
  1. Build log4cxx lib. This can be done using autotools, just follow the instructions at http://logging.apache.org/log4cxx/building/autotools.html.
  1. Extract the php source file to someplace in you disk. Here we assume that the files are extracted to /tmp/php-5.2.10
```
  cd /tmp
  tar -xzfv php-5.2.10.tar.gz
```
  1. Checkout and create the logger dir inside the php ext dir
```
  cd /tmp/php-5.2.10/ext
  svn checkout http://php-logger-extension.googlecode.com/svn/trunk/ logger
```
  1. Compile the extension. Here you will nedd to know the log4cxx lib dir (usually is /usr/local/lib)
```
  cd logger
  phpize
  ./configure --with-logger=[log4cxx lib dir]
  make
  make install
```
  1. If everything is fine you have php\_logger.so installed. Now you need to change your php.ini to load the extension, just add extension=logger.so to the list of load extensions.
  1. If you are starting to use the extension check the examples on how to configure the logger for use.

## Building on Windows Xp ##
We are compiling a how-to to build on windows with visual c 6 and 9.
Windows pre-compiled zip packages are available for download, anyway if you need to compile from source (e.g. for local changes) then please come back later.

