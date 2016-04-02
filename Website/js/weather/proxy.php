<?php
	
	if(!isset($_GET['url'])) die();
$url = $_GET['url']."?units=".$_GET["units"];
//echo $url;
$url = file_get_contents($url);
print_r($url);
	?>