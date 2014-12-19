<?php
	$datadir = "data/images/";
	$response=new stdClass();
	
	// get json
	$markup = file_get_contents('php://input');	
	$parsed = json_decode($markup);
	
	$indx = count($parsed->objects);
	$indx--;
	
	$uploadID = $parsed->objects[$indx]->UPLOADID;
	$imageID = $parsed->objects[$indx]->IMAGEID;
	$markupCount = $parsed->objects[$indx]->markup_count;
	
	$datadir .= $uploadID."/";
	
	$markup_path = $datadir."markup_".$imageID."/markup_".$markupCounter.".txt";
	
	// remove old markup
	if (unlink($markup_path)) {
	    if (!file_put_contents($markup_path, json_encode($parsed))) {
            $response->response = "Update Failed!";
        } else {
            $response->response = "Updated!";
        }
	} else {
	    $response->response = "File Missing!";
	}
        
    $response->markupCounter = $markupCounter;

	echo json_encode($response);
?>