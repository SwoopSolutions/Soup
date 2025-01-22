<?php
// https://curl.se/docs/caextract.html
$file = file_get_contents("cacert.pem"); // https://curl.se/ca/cacert.pem
$cert = false;
foreach(explode("\n", $file) as $line)
{
	$line = trim($line);
	if ($line == "-----BEGIN CERTIFICATE-----")
	{
		echo "\"";
		$cert = true;
		continue;
	}
	if ($line == "-----END CERTIFICATE-----")
	{
		echo "\",\n";
		$cert = false;
		continue;
	}
	if ($cert)
	{
		echo $line;
	}
}
