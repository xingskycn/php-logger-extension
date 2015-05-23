# Introduction #
Here goes some simple examples on using this extension within applications.
It is recommended that you also read the [short introduction to log4cxx](http://logging.apache.org/log4cxx/).

# Details #
The first thing to do is to configure the logger through a log4j.properties file.

Following is an example of such file:
```
log4j.rootLogger=DEBUG, AP1
log4j.appender.AP1=org.apache.log4j.RollingFileAppender
log4j.appender.AP1.File=/tmp/myapp.log
log4j.appender.AP1.MaxFileSize=100KB
log4j.appender.AP1.MaxBackupIndex=5
log4j.appender.AP1.layout=org.apache.log4j.PatternLayout
log4j.appender.AP1.layout.ConversionPattern=%p %t %c - %m%n
```

This file configures the framework to work on DEBUG level and creates an appender named AP1.
The AP1 appender is then configured to be an rolling file appender, writing the messages to /tmp/myapp.log, the appender will hold a backup of 5 files each one with 100kb. It alsos configures the appender pattern to show the thread id, the level and others.

After writing the properties file you just need to load it with LoggerPropertyConfigurator class and you will be able to write the messages to /tmp/myapp.log.

```
<?php
LoggerPropertyConfigurator::configure("log4j.properties");

class Foo {
  
  public function bar() {
    $logger = LoggerManager::getLogger(__METHOD__);
    $logger->debug('entering bar');
    // ... do something and set the $result var to an integer
    $logger->debug('leaving bar with result '.$result);
  }
}

$foo = new Foo();
$foo->bar();
```

Here is an example of myapp.log file contents :

```
DEBUG 0xb7a566c0 Foo::Bar - entering bar
DEBUG 0xb7a566c0 Foo::Bar - leaving bar with result 1
```