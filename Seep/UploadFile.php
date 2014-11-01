<?php
// summary
// Test file to handle image uploads (remove the image size check to upload non-images)
//
// This file handles both Flash and HTML uploads
//
// NOTE: This is obviously a PHP file, and thus you need PHP running for this to work
// NOTE: Directories must have write permissions
// NOTE: This code uses the GD library (to get image sizes), that sometimes is not pre-installed in a
// standard PHP build.
//
require("includes/cLOG.php");
require("includes/UUID.php");

function findTempDirectory()
  {
    if(isset($_ENV["TMP"]) && is_writable($_ENV["TMP"])) return $_ENV["TMP"];
    elseif( is_writable(ini_get('upload_tmp_dir'))) return ini_get('upload_tmp_dir');
    elseif(isset($_ENV["TEMP"]) && is_writable($_ENV["TEMP"])) return $_ENV["TEMP"];
    elseif(is_writable("/tmp")) return "/tmp";
    elseif(is_writable("/windows/temp")) return "/windows/temp";
    elseif(is_writable("/winnt/temp")) return "/winnt/temp";
    else return null;
  }

function trace($txt, $isArray=false){
    //creating a text file that we can log to
    // this is helpful on a remote server if you don't
    //have access to the log files
    //
    $log = new cLOG("includes/upload.txt", false);
    //$log->clear();
    if ($isArray) {
        $log->printr($txt);
    } else {
        $log->write($txt);
    }

    //echo "$txt<br>";
}

function getImageType($filename) {
    return strtolower(substr(strrchr($filename,"."),1));
}

include 'includes/WideImage/lib/WideImage.php';
#include 'includes/maptiler.php';

function readGPSinfoEXIF($exif, &$exif_data) {
// how different is each device?
    if (!$exif || $exif['GPS']['GPSLatitude'] == '') {
        return "| no GPS";
    } else {
        $lat_ref = $exif['GPS']['GPSLatitudeRef'];
        $lat = $exif['GPS']['GPSLatitude'];
        list($num, $dec) = explode('/', $lat[0]);
        $lat_s = $num / $dec;
        list($num, $dec) = explode('/', $lat[1]);
        $lat_m = $num / $dec;
        list($num, $dec) = explode('/', $lat[2]);
        $lat_v = $num / $dec;

        $lon_ref = $exif['GPS']['GPSLongitudeRef'];
        $lon = $exif['GPS']['GPSLongitude'];
        list($num, $dec) = explode('/', $lon[0]);
        $lon_s = $num / $dec;
        list($num, $dec) = explode('/', $lon[1]);
        $lon_m = $num / $dec;
        list($num, $dec) = explode('/', $lon[2]);
        $lon_v = $num / $dec;

        $lat_int = ($lat_s + $lat_m / 60.0 + $lat_v / 3600.0);
        // check orientaiton of latitude and prefix with (-) if S
        $lat_int = ($lat_ref == "S") ? '-' . $lat_int : $lat_int;

        $lon_int = ($lon_s + $lon_m / 60.0 + $lon_v / 3600.0);
        // check orientation of longitude and prefix with (-) if W
        $lon_int = ($lon_ref == "W") ? '-' . $lon_int : $lon_int;
        
        $data = explode("/", $exif['GPS']['GPSAltitude']);
        
        $altitude = (int) $data[0] / (int) $data[1];
        
        if (!$exif['GPS']['GPSAltitudeRef']) {
        	$altitude = -1*$altitude;
        }
        
        $data = explode("/", $exif['GPS']['GPSImgDirection']);
  
        $heading = (int) $data[0] / (int) $data[1];

        if ($exif['GPS']['GPSImgDirectionRef']!="T") {
        	$heading = $heading-9.77; // might need to calculate this, don't know if any cameras use it
        }
        
        $gps_int = array("latitude"=>$lat_int, "longitude"=>$lon_int,
         "gpsAltitude"=>$altitude,
         "gpsHeading"=>$heading);

        $exif_data = array_merge($exif_data, $gps_int);
        return "| GPS";
    }
}

function tileImage($upload_path, $hash, $imageNumber, &$exif_data) {
    $message = "";

    for ($n=0;$n<$imageNumber;$n++) {

        // sized path
        $uploadSized_path = $upload_path."/sized_".$hash[$n];
        mkdir($uploadSized_path, 0775);
        $uploadSized_path = $uploadSized_path."/";

        // size 3 images ?? what if smaller, don't size up
        $width=100;
        $height=75;
        WideImage::load($upload_path.$hash[$n].".jpg")->resize($width, $height)->saveToFile($uploadSized_path.$hash[$n]."_thumb.jpg");

        $width=200;
        $height=150;
        WideImage::load($upload_path.$hash[$n].".jpg")->resize($width, $height)->saveToFile($uploadSized_path.$hash[$n]."_small.jpg");

        $width=400;
        $height=300;
        WideImage::load($upload_path.$hash[$n].".jpg")->resize($width, $height)->saveToFile($uploadSized_path.$hash[$n]."_medium.jpg");

        $width=1280;
        $height=960;
        WideImage::load($upload_path.$hash[$n].".jpg")->resize($width, $height)->saveToFile($uploadSized_path.$hash[$n]."_large.jpg");

        // tile image
        $uploadTile_path = $upload_path."/tiles_".$hash[$n];
        mkdir($uploadTile_path, 0775);
        $uploadTile_path = $uploadTile_path."/";
        
        $map_tiler = "imgcnv -i ".$upload_path.$hash[$n].".jpg"." -o ".$uploadTile_path."tiles.jpg -t jpeg -tile 256 -verbose";
        
        //execute
        $return_var = 0;
        system($map_tiler, &$return_var);
        $message += "imgcnv".$return_var."|".$map_tiler;
        
        // exif data
        $exif = exif_read_data($upload_path.$hash[$n].".jpg", 'EXIF');

        if (!$exif) {
            $message += "| No header data found ";
        } else {
            $exif = exif_read_data($upload_path.$hash[$n].".jpg", 0, true);

            $exif_data[$n]["FileName"] = $exif["FILE"]["FileName"];
            
            $timestamp = $exif["FILE"]["FileDateTime"];
            $exif_data[$n]["FileDate"] = date('m-d-Y', $timestamp);
            $exif_data[$n]["FileTime"] = date('H:i:s', $timestamp);
            
            $data = explode(" ", $exif["EXIF"]["DateTimeOriginal"]);
            
            $exif_data[$n]["Date"] = implode("/", explode(":", $data[0]));
            $exif_data[$n]["Time"] = $data[1];
            
            $message += readGPSinfoEXIF($exif, $exif_data[$n]);
        }
    }

    return $message;
}

trace("---------------------------------------------------------");

//
//
// EDIT ME: According to your local directory structure.
// NOTE: Folders must have write permissions
//
$upload_path = "data/images/"; // where image will be uploaded, relative to this file
$download_path = "Seep/data/images/"; // same folder as above, but relative to the HTML file
$exif = array();

//
// NOTE: maintain this path for JSON services
//
require("includes/JSON.php");
$json = new Services_JSON();

//
// Determine if this is a Flash upload, or an HTML upload
//
//

// First combine relavant postVars
$postdata = array();
$htmldata = array();
$data = "";
$imageHash = array();

trace("POSTDATA: " . count($_FILES) . " FILES");

foreach ($_POST as $nm => $val) {
    $data .= $nm ."=" . $val . ","; // string for flash
    $postdata[$nm] = $val; // array for html
}

trace($postdata, true);

$hash = $postdata["UPLOADIDImageUpload"];

$upload_path = $upload_path."/".$hash;
mkdir($upload_path, 0775);

$upload_path = $upload_path."/";

foreach ($_FILES as $nm => $val) {
    trace(" file: ".$nm ."=" . $val);
}

foreach ($_GET as $nm=> $val) {
    trace($nm ."=" . $val);
}

$fieldName = "flashUploadFiles";//Filedata";

if ( isset($_FILES[$fieldName]) || isset($_FILES['uploadedfileFlash'])) {
    //
    // If the data passed has $fieldName, then it's Flash.
    // NOTE: "Filedata" is the default fieldname, but we're using a custom fieldname.
    // The SWF passes one file at a time to the server, so the files come across looking
    // very much like a single HTML file. The SWF remembers the data and returns it to
    // Dojo as an array when all are complete.
    //
    trace("returnFlashdata....");

    trace("");
    trace("ID:");

    trace("Flash POST:");
    trace($_POST, true);


    $returnFlashdata = true; //for dev

    if ( isset($_FILES[$fieldName])) {
    // backwards compat - FileUploader
    trace("FILES:");
    trace($_FILES[$fieldName], true);
    $m = move_uploaded_file($_FILES[$fieldName]['tmp_name'], $upload_path . $_FILES[$fieldName]['name']);
    $name = $_FILES[$fieldName]['name'];

    } else {
        // New fieldname - Uploader
        trace("FILES:");
        trace($_FILES['uploadedfileFlash'], true);
        $m = move_uploaded_file($_FILES['uploadedfileFlash']['tmp_name'], $upload_path . $_FILES['uploadedfileFlash']['name']);
        $name = $_FILES['uploadedfileFlash']['name'];
    }

    $file = $upload_path . $name;

    try{
        list($width, $height) = getimagesize($file);
    } catch(Exception $e){
        $width=0;
        $height=0;
    }

    $type = getImageType($file);
    trace("file: " . $file ." ".$type." ".$width);
    // Flash gets a string back:

    //exit;

    $data .='file='.$file.',name='.$name.',width='.$width.',height='.$height.',type='.$type;
    if($returnFlashdata){
        trace("returnFlashdata:\n=======================");
        trace($data);
        trace("=======================");

        // process image
        trace(tileImage($upload_path, $imageHash, 1, $exif));

        // get values later
        // $data .= 'exif='.$exif;

        // echo sends data to Flash:
        echo($data);

        // return is just to stop the script:
        return;
    }

} elseif ( isset($_FILES['uploadedfile0']) ) {
    //
    // Multiple files have been passed from HTML
    //
    $cnt = 0;
    trace("IFrame multiple POST:");
    trace($postdata, true);

    $_post = $htmldata;
    $htmldata = array();

    $n=1;

    while(isset($_FILES['uploadedfile'.$cnt])){
        trace("IFrame multiple POST");
        $imageHash[$cnt] = UUID::v4();
        $moveName = $imageHash[$cnt].".jpg";
        $moved = move_uploaded_file($_FILES['uploadedfile'.$cnt]['tmp_name'], $upload_path . moveName);
        trace("moved:" . $moved ." ". $moveName);

        if ($moved) {
            $name = moveName;
            $file = $upload_path . $name;
            $type = getImageType($file);

            try {
                list($width, $height) = getimagesize($file);
            } catch (Exception $e) {
                $width=0;
                $height=0;
            }

            trace("file: " . $file );

            $_post['file'] = $file;
            $_post['name'] = $name;
            $_post['width'] = $width;
            $_post['height'] = $height;
            $_post['type'] = $type;
            $_post['uploadType'] = $postdata['uploadType'];
            $_post['size'] = filesize($file);
            $_post['additionalParams'] = $postdata;
            trace($_post['additionalParams'], true);

            $htmldata[$cnt] = $_post;

            foreach($postdata as $key => $value){
                //$htmldata[ $key ] = $value;
            }

        } elseif (strlen($_FILES['uploadedfile'.$cnt]['name'])) {
 	        $htmldata[$cnt] = array("Message" => "File could not be moved: ".$_FILES['uploadedfile'.$cnt]['name']);
        }

        $cnt++;
    }

    trace("HTML multiple POST done:");

    foreach($htmldata as $key => $value){
        trace($value, true);
    }

    // process image
    trace(tileImage($upload_path, $imageHash, $cnt, $exif));

    $htmldata['exif'] = $exif;

} elseif ( isset($_POST['uploadedfiles']) ) {
  trace("HTML5 multi file input... CAN'T ACCESS THIS OBJECT! (POST[uploadedfiles])");
  trace(count($_POST['uploadedfiles'])." ");
} elseif ( isset($_FILES['uploadedfiles']) ) {
    //
    // If the data passed has 'uploadedfiles' (plural), then it's an HTML5 multi file input.
    //
    $cnt = 0;
    trace("HTML5 multi file input");
    //trace($_FILES, true);
    //print_r($_FILES);
    $_post = $postdata;
    trace("POST DATA:::");
    trace($_post, true);
    $htmldata = array();
    $len = count($_FILES['uploadedfiles']['name']);
    //
    // Ugh. The array passed from HTML to PHP is fugly.
    //

    //print_r($_FILES['uploadedfiles']);
    $dir = scandir($upload_path);
    $n=1;
    for ($s=2;$s<count($dir);$s++) {
        $temp = explode("_", $dir[$s]);
        if ($temp[0]==$hash) {
            $n++;
        }
    }

    for($i=0;$i<$len;$i++){
        $imageHash[$i] = UUID::v4();
        $moveName = $imageHash[$i].".jpg";
        $moved = move_uploaded_file($_FILES['uploadedfiles']['tmp_name'][$i], $upload_path . $moveName);
        trace("moved:" . $moved ." ". $_FILES['uploadedfiles']['name'][$i]);

        if ($moved) {
            $name = $moveName;
            $file = $upload_path . $name;
            $type = getImageType($file);

            try{
                list($width, $height) = getimagesize($file);
            } catch(Exception $e){
                error_log("NO EL MOVEO: " . $name);
                $width=0;
                $height=0;
                $_post['filesInError'] = $name;
            }

            if(!$width){
                $_post['filesInError'] = $name;
                $width=0;
                $height=0;
            }

            trace("file: " . $file ." size: " . $width." ".$height);

            $_post['file'] = $file;
            $_post['name'] = $name;
            $_post['width'] = $width;
            $_post['height'] = $height;
            $_post['type'] = $type;
            $_post['size'] = filesize($file);

            //$_post['additionalParams'] = $postdata;
            //trace($_post, true);

            $htmldata[$cnt] = $_post;
            $n++;
        } elseif(strlen($_FILES['uploadedfiles']['name'][$i])) {
            $htmldata[$cnt] = array("Message" => "File could not be moved: ".$_FILES['uploadedfiles']['name'][$i]);
        }

        $cnt++;
    }

    $htmldata['Message'] = "Submit Successful";

    // process image
    trace(tileImage($upload_path, $imageHash, $cnt, $exif));

    $htmldata['exif'] = $exif;

    $data = $json->encode($htmldata);
    trace($data);

    print $data;
    return $data;

} elseif ( isset($_FILES['uploadedfile']) ) {
    //
    // If the data passed has 'uploadedfile', then it's HTML.
    // There may be better ways to check this, but this *is* just a test file.
    //
    $m = move_uploaded_file($_FILES['uploadedfile']['tmp_name'], $upload_path . $_FILES['uploadedfile']['name']);

    trace("HTML single POST:");
    trace($postdata, true);

    $name = $_FILES['uploadedfile']['name'];
    $file = $upload_path . $name;
    $type = getImageType($file);

    try{
        list($width, $height) = getimagesize($file);
    } catch(Exception $e){
        $width=0;
        $height=0;
    }

    trace("file: " . $file );

    $htmldata['file'] = $file;
    $htmldata['name'] = $name;
    $htmldata['width'] = $width;
    $htmldata['height'] = $height;
    $htmldata['type'] = $type;
    $htmldata['uploadType'] = $uploadType;
    $htmldata['size'] = filesize($file);
    $htmldata['additionalParams'] = $postdata;
    $htmldata['Message'] = "Submit Successful";

    // process image
    trace(tileImage($upload_path, $imageHash, 1, $exif));

    $htmldata['exif'] = $exif;

    $data = $json->encode($htmldata);
    trace($data);

    print $data;
    return $data;
} elseif (isset($_GET['rmFiles'])) {
    trace("DELETING FILES" . $_GET['rmFiles']);
    $rmFiles = explode(";", $_GET['rmFiles']);

    foreach($rmFiles as $f){
        if($f && file_exists($f)){
            trace("deleted:" . $f. ":" .unlink($f));
        }
    }

}else{
    trace("IMROPER DATA SENT... $_FILES:");
    trace($_FILES);
    $htmldata = array("Message" => "Submit Successful - No Images Sent");
}

//HTML gets a json array back:
$data = $json->encode($htmldata);
trace("Json Data Returned:");
trace($data);
echo $data;
//return $data;
?>
