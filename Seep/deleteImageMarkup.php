<?php
	$datadir = "data/images/";
	$response=new stdClass();
	
	// get json
	$markup = file_get_contents('php://input');	
	$parsed = json_decode($markup);
	
	$uploadID = $parsed->UPLOADID;	
	$imageID = $parsed->IMAGEID;
	$markupCounter =  $parsed->markupCounter;
	
	$datadir .= $uploadID."/";
	
	$markup_path = $datadir."markup_".$imageID."/markup_".$markupCounter.".txt";
	
	if (unlink($markup_path)) {
        $response->response = "Done";
	} else {
	    $response->response = "File Missing!";
	}
        
    $response->markupCounter = $markupCounter;

	echo json_encode($response);
?>