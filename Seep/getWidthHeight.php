<?php
require("includes/JSON.php");
$json = new Services_JSON();

$uploadId = $_POST["uploadid"];
$imageId = $_POST["imageid"];
$path = "data/images/"; // where image is, relative to this file

$image_path = $path.$uploadId."/".$imageId.".jpg";

$size = getimagesize($image_path);

$data["uploadid"] = $uploadId;
$data["imageid"] = $imageId;
$data["width"] = $size[0];
$data["height"] = $size[1];

$data = $json->encode($data);
echo $data;
?>