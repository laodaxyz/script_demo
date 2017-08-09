<?php
function transByte($size) {
	$arr = array ("B", "KB", "MB", "GB", "TB", "EB" );
	$i = 0;
	while ( $size >= 1024 ) {
		$size /= 1024;
		$i ++;
	}
	return round ( $size, 2 ) . $arr [$i];
}

?>