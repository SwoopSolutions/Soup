<?php
// https://curl.se/docs/caextract.html
$file = file_get_contents("cacert.pem"); // https://curl.se/ca/cacert.pem
$all_certs = "";
$num_certs = 0;
$cert = null;
foreach(explode("\n", $file) as $line)
{
	$line = trim($line);
	if ($line == "-----BEGIN CERTIFICATE-----")
	{
		$cert = "";
		continue;
	}
	if ($line == "-----END CERTIFICATE-----")
	{
		$decoded = base64_decode($cert);
		(strlen($decoded) <= 0xFFFF) or die("Cert to big");
		$all_certs .= pack("v", strlen($decoded)).$decoded;
		++$num_certs;
		$cert = null;
		continue;
	}
	if ($cert !== null)
	{
		$cert .= $line;
	}
}
($num_certs <= 0xFFFF) or die("Too many certs");
$all_certs = pack("v", $num_certs).$all_certs;
$bin_str = gzdeflate($all_certs);
file_put_contents("../cacerts.hpp", "static const char cacerts[] = { '\\x".join("', '\\x", array_map("dechex", array_map("ord", str_split($bin_str))))."' };");
