<?php
	$datadir = "data/images/";
	$response=new stdClass();
	$count = 0;
	
	// get json
	$markup = file_get_contents('php://input');
	
	$parsed = json_decode($markup);
	
	$uploadID = $parsed->UPLOADID;
	
	$imageID = $parsed->IMAGEID;
	
	$datadir .= $uploadID."/";
	
	$markup_path = $datadir."markup_".$imageID;
	
    $files = scandir($markup_path);
    $count = count($files)-2;
        
    $response->response = "Found";
    $response->markup = array();
    $markup = "";
    
    foreach (scandir($markup_path) as $file) {
        if ('.' === $file) continue;
        if ('..' === $file) continue;

        $markup = file_get_contents($markup_path."/".$file);
        $response->markup[] = json_decode($markup);
    } 

	echo json_encode($response);
?>