<?php
  include "functions/gzip.php";
	$url = $_GET["url"];
	echo file_get_contents($url);
	//echo get_url($url);
	if(get_url($url) == ""){
		echo file_get_contents($url);
	}
?>
