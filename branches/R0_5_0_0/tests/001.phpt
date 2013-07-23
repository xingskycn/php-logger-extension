--TEST--
File Appender - test if the logger is able to append to a file
--FILE--
<?php
$sDir = dirname(__FILE__). DIRECTORY_SEPARATOR;
$sDir = str_replace('\\', '\\\\', $sDir);

$sLogFile = $sDir."test_logger.log";
$sLogText = 'test test test 12345';
$sPropertyFile = $sDir."log4j.properties";
$sPropertyContents = <<<EOT
log4j.rootLogger=DEBUG, TEST
log4j.appender.TEST=org.apache.log4j.RollingFileAppender
log4j.appender.TEST.File=$sLogFile
log4j.appender.TEST.MaxFileSize=100KB
log4j.appender.TEST.MaxBackupIndex=1
log4j.appender.TEST.layout=org.apache.log4j.PatternLayout
log4j.appender.TEST.layout.ConversionPattern=%p %t %c - %m%n

EOT;
$f = fopen($sPropertyFile, "w");
fwrite($f, $sPropertyContents);
fclose($f);

LoggerPropertyConfigurator::configure($sPropertyFile);
$logger = LoggerManager::getLogger("Some::Method");
$logger->debug($sLogText);
$logger = null;

$aDebugLines = file($sLogFile);
$sDebugLine = $aDebugLines[0];
if (strpos($sDebugLine, $sLogText) === false) {
	print "FAIL";
} else {
	print "PASS";
}
?>
--EXPECT--
PASS
--CLEAN--
<?php
$sDir = dirname(__FILE__).'/';
$sLogFile = $sDir."test_logger.log";
$sPropertyFile = $sDir."log4j.properties";
unlink($sLogFile);
unlink($sPropertyFile);
?>