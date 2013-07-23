--TEST--
getEffectiveLevel - test reading the logger level
--FILE--
<?php
function getPropertyContents($sLevel, $sLogFile) {
	$sPropertyContents = <<<EOT
log4j.rootLogger=$sLevel, TEST
log4j.appender.TEST=org.apache.log4j.RollingFileAppender
log4j.appender.TEST.File=$sLogFile
log4j.appender.TEST.MaxFileSize=100KB
log4j.appender.TEST.MaxBackupIndex=1
log4j.appender.TEST.layout=org.apache.log4j.PatternLayout
log4j.appender.TEST.layout.ConversionPattern=%p %t %c - %m%n

EOT;
	return $sPropertyContents;
}

function changeLogLevel($sLevel) {
	$sDir = dirname(__FILE__) . DIRECTORY_SEPARATOR;
	$sDir = str_replace('\\', '\\\\', $sDir);
	$sPropertyFile = $sDir."log4j.properties";
	$sLogFile = $sDir."test_logger.log";

	$f = fopen($sPropertyFile, "w");
	fwrite($f, getPropertyContents($sLevel, $sLogFile));
	fclose($f);

	/* ensure existent log file */
	$f = fopen($sLogFile, "w");
	fwrite($f,"");
	fclose($f);

	LoggerPropertyConfigurator::configure($sPropertyFile);
}

changeLogLevel("TRACE");
$logger = LoggerManager::getLogger("Some::Method");
var_dump($logger->getEffectiveLevel());

changeLogLevel("DEBUG");
$logger = LoggerManager::getLogger("Some::Method");
var_dump($logger->getEffectiveLevel());

changeLogLevel("WARN");
$logger = LoggerManager::getLogger("Some::Method");
var_dump($logger->getEffectiveLevel());

changeLogLevel("ERROR");
$logger = LoggerManager::getLogger("Some::Method");
var_dump($logger->getEffectiveLevel());

changeLogLevel("FATAL");
$logger = LoggerManager::getLogger("Some::Method");
var_dump($logger->getEffectiveLevel());

changeLogLevel("INFO");
$logger = LoggerManager::getLogger("Some::Method");
var_dump($logger->getEffectiveLevel());



?>
--EXPECT--
string(5) "TRACE"
string(5) "DEBUG"
string(4) "WARN"
string(5) "ERROR"
string(5) "FATAL"
string(4) "INFO"
--CLEAN--
<?php
$sDir = dirname(__FILE__).'/';
$sPropertyFile = $sDir."log4j.properties";
$sLogFile = $sDir."test_logger.log";
unlink($sLogFile);
unlink($sPropertyFile);
?>