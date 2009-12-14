--TEST--
location - test if the logger points to proper php location file, line and function
--FILE--
<?php
$sDir = dirname(__FILE__). DIRECTORY_SEPARATOR;
$sDir = str_replace('\\', '\\\\', $sDir);

$sLogFile = $sDir."test_logger.log";
$sLogText = 'test';
$sPropertyFile = $sDir."log4j.properties";
$sPropertyContents = <<<EOT
log4j.rootLogger=TRACE, TEST
log4j.appender.TEST=org.apache.log4j.RollingFileAppender
log4j.appender.TEST.File=$sLogFile
log4j.appender.TEST.MaxFileSize=100KB
log4j.appender.TEST.MaxBackupIndex=1
log4j.appender.TEST.layout=org.apache.log4j.PatternLayout
log4j.appender.TEST.layout.ConversionPattern=%c %F:%L - %m%n

EOT;
$f = fopen($sPropertyFile, "w");
fwrite($f, $sPropertyContents);
fclose($f);

LoggerPropertyConfigurator::configure($sPropertyFile);

function foo() {
	$logger = LoggerManager::getLogger(__FUNCTION__);
	global $sLogText;
	$logger->debug($sLogText);
	$logger->trace($sLogText);
	$logger->info($sLogText);
	$logger->warn($sLogText);
	$logger->error($sLogText);
	$logger->fatal($sLogText);
}

foo();

$aDebugLines = file($sLogFile);
$iLine = 30;
foreach ($aDebugLines as $sDebugText) {
	$sExpected = sprintf("foo %s:%2d - test%s", __FILE__, $iLine++, PHP_EOL);
	$iDiff = strcmp($sExpected, $sDebugText);
	if ($iDiff === 0) {
		print "PASS\n";
	} else {
		print "FAIL\n";
		//print "Expected: ".$sExpected."\n";
		//print "Found   : ".$sDebugText."\n";
	}
}

?>
--EXPECT--
PASS
PASS
PASS
PASS
PASS
PASS
--CLEAN--
<?php
$sDir = dirname(__FILE__).'/';
$sPropertyFile = $sDir."log4j.properties";
$sLogFile = $sDir."test_logger.log";
unlink($sLogFile);
unlink($sPropertyFile);
?>