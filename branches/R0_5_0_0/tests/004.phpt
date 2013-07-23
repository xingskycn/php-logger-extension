--TEST--
location - test if the logger points to proper php location file, line, class and method
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

$iLine = 0;

Class Foo {
	public function bar() {
		global $sLogText;
		global $iLine;
		$logger = LoggerManager::getLogger(__METHOD__);
		$logger->debug($sLogText);
		$logger->trace($sLogText);
		$logger->info($sLogText);
		$logger->warn($sLogText);
		$logger->error($sLogText);
		$logger->fatal($sLogText);
		$iLine = __LINE__ - 6;
	}
}

$f = new Foo();
$f->bar(); 

$aDebugLines = file($sLogFile);
foreach ($aDebugLines as $sDebugText) {
	$sExpected = sprintf("Foo::bar %s:%2d - test%s", __FILE__, $iLine++, PHP_EOL);
	$iDiff = strcmp($sExpected, $sDebugText);
	if ($iDiff === 0) {
		print "PASS\n";
	} else {
		print "FAIL\n";
		print "Expected (string ".strlen($sExpected).") ".$sExpected."\n";
		print "Found (string ".strlen($sDebugText).") ".$sDebugText."\n";
		print "Expected: ".bin2hex($sExpected)."\n";
		print "Found   : ".bin2hex($sDebugText)."\n";
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