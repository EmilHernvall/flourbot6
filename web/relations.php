<?php

function F($v, $l, $u, $max)
{
	$m = ($l + $u) / 2;
	$k = $max / (($l - $m) * ($u - $m));
	return max(0, $k * ($l - $v) * ($u - $v));
}

function imagelinethick($image, $x1, $y1, $x2, $y2, $color, $thick = 1)
{
    if ($thick == 1) {
        return imageline($image, $x1, $y1, $x2, $y2, $color);
    }
    $t = $thick / 2 - 0.5;
    if ($x1 == $x2 || $y1 == $y2) {
        return imagefilledrectangle($image, round(min($x1, $x2) - $t), round(min($y1, $y2) - $t), round(max($x1, $x2) + $t), round(max($y1, $y2) + $t), $color);
    }
    $k = ($y2 - $y1) / ($x2 - $x1); //y = kx + q
    $a = $t / sqrt(1 + pow($k, 2));
    $points = array(
        round($x1 - (1+$k)*$a), round($y1 + (1-$k)*$a),
        round($x1 - (1-$k)*$a), round($y1 - (1+$k)*$a),
        round($x2 + (1+$k)*$a), round($y2 - (1-$k)*$a),
        round($x2 + (1-$k)*$a), round($y2 + (1+$k)*$a),
    );
    imagefilledpolygon($image, $points, 4, $color);
    return imagepolygon($image, $points, 4, $color);
}

$width = 1500;
$height = $width;
$size = 10;
$font = "verdana.ttf";

include "db.php";

$conn = mysql_connect(DB_HOST, DB_USERNAME, DB_PASSWORD) or die(mysql_error());
mysql_select_db(DB_DB, $conn) or die(mysql_error());

$channel = array_key_exists("channel", $_GET) ? $_GET["channel"] : "";
if (!$channel) {
	echo "You might want to try one of the following channels:<br />";
	echo "<ul>";
	$sql = "SELECT DISTINCT address_channel FROM address";
	$qry = mysql_query($sql) or die(mysql_error());
	while ($arr = mysql_fetch_assoc($qry)) {
		printf('<li><a href="relations.php?channel=%s">%s</a></li>', urlencode(substr($arr["address_channel"],1)), $arr["address_channel"]);
	}
	echo "</ul>";
	exit;
}

// Yes, this is a horrible hack.
$sql = "CREATE TEMPORARY TABLE %s 
SELECT address_source, sum(address_count) sum FROM address
WHERE address_channel = '%s'
GROUP BY address_source
ORDER BY sum DESC
LIMIT 50";
mysql_query(sprintf($sql, "tmp", "#".mysql_real_escape_string($channel)), $conn) or die(mysql_error());
mysql_query(sprintf($sql, "tmp2", "#".mysql_real_escape_string($channel)), $conn) or die(mysql_error());

$sql = "SELECT a.address_source, a.address_target, (b.address_count + a.address_count) address_count
FROM `address` a
INNER JOIN address b ON a.address_source = b.address_target
AND b.address_source = a.address_target
AND b.address_channel = a.address_channel
INNER JOIN tmp ON tmp.address_source = a.address_source
INNER JOIN tmp2 ON tmp2.address_source = a.address_target
WHERE a.address_channel = '%s'
HAVING address_count > 2
ORDER BY address_source, address_target";

$qry = mysql_query(sprintf($sql, "#".mysql_real_escape_string($channel)), $conn) or die(mysql_error());

$data = array();
$maxCount = 1;
while ($arr = mysql_fetch_assoc($qry)) {
	$source = $arr["address_source"];
	$target = $arr["address_target"];
	$count = $arr["address_count"];

	$data[$source][$target] = $count;

	$maxCount = max($maxCount, $count);
}

if ($channel == "sverok") {
	$newOrder = array_map("trim", file("friends.order.txt"));

	$newData = array();
	foreach ($newOrder as $order) {
		if (!array_key_exists($order, $data)) {
			continue;
		}
		$newData[$order] = $data[$order];
	}

	$data = $newData;
}

$count = count($data);
if ($count < 2) {
	echo "Not enough data for this channel!\n";
	exit;
}

$img = imagecreatetruecolor($width, $height);
imageantialias($img, true);

$white = imagecolorallocate($img, 255, 255, 255);
$black = imagecolorallocate($img, 0, 0, 0);
$red = imagecolorallocate($img, 255, 0, 0);

imagefill($img, 0, 0, $white);

imagettftext($img, $size, 0, 0, $size, $black, $font, sprintf("People addressing each other in #%s, generated on %s.", $channel, date("Y-m-d H:i:s")));

$eight = $width / 8;
for ($i = 0; $i < $width; $i++) {
	$r = F($i, 4 * $eight, 10 * $eight, 255);
	$g = F($i, 1 * $eight, 7 * $eight, 255);
	$b = F($i, -2 * $eight, 4 * $eight, 255);
	$color = imagecolorallocate($img, $r, $g, $b);
	imageline($img, $i, $height - 10, $i, $height, $color);
}

$pie = 360 / $count;

$centerX = $width / 2;
$centerY = $height / 2;

$v = 0;
$angles = array();
foreach ($data as $nick => $rels) {
	$x = $centerX + 0.7 * $centerX * cos(deg2rad($v));
	$y = $centerY + 0.7 * $centerY * sin(deg2rad($v));

	$text = sprintf("%s (%d)", $nick, count($rels));

	if ($v > 90 && $v < 270) {
		$v2 = $v - 180;
		$res = imagettfbbox($size, 0, $font, $text);
		$textWidth = $res[2];
		$xText = $centerX + (0.75 * $centerX + $textWidth) * cos(deg2rad($v));
		$yText = $centerY + (0.75 * $centerY + $textWidth) * sin(deg2rad($v));
		imagettftext($img, $size, -$v2, $xText, $yText, $black, $font, $text);
	} else {
		$xText = $centerX + 0.75 * $centerX * cos(deg2rad($v));
		$yText = $centerY + 0.75 * $centerY * sin(deg2rad($v));
		imagettftext($img, $size, -$v, $xText, $yText, $black, $font, $text);
	}

	$angles[$nick] = $v;

	$v += $pie;
}

foreach ($data as $nick => $rels) {
	$sourceAngle = $angles[$nick];

	$sourceX = $centerX + 0.7 * $centerX * cos(deg2rad($sourceAngle));
	$sourceY = $centerY + 0.7 * $centerY * sin(deg2rad($sourceAngle));

	foreach ($rels as $rel => $count) {
		if (strcasecmp($nick, $rel) < 0) {
			continue;
		}

		$targetAngle = $angles[$rel];

		$targetX = $centerX + 0.7 * $centerX * cos(deg2rad($targetAngle));
		$targetY = $centerY + 0.7 * $centerY * sin(deg2rad($targetAngle));

		$thickness = max(1, round($count * 5 / $maxCount));

		$colorValue = $count * $width/$maxCount;
		$r = F($colorValue, 4 * $eight, 10 * $eight, 255);
		$g = F($colorValue, 1 * $eight, 7 * $eight, 255);
		$b = F($colorValue, -2 * $eight, 4 * $eight, 255);
		$color = imagecolorallocate($img, $r, $g, $b);

		imagelinethick($img, $sourceX, $sourceY, $targetX, $targetY, $color, $thickness);
	}
}

header("Content-type: image/jpeg");
imagejpeg($img, NULL, 90);
imagedestroy($img);
