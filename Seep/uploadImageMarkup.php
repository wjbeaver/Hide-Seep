<?php
	$datadir = "data/images/";
	$response=new stdClass();
	
	// get json
	$markup = file_get_contents('php://input');
	
	$parsed = json_decode($markup);
	
	$source = $parsed->objects[0]->source;
	
	$imageCount = $parsed->objects[0]->image_count;
	
	$markupCount = $parsed->objects[0]->markup_count;
	
	$datadir .= $source."/";
	
	$markup_path = $datadir."/markup_".$imageCount;
	
	// check to see if exists
	if (!file_exists ( $markup_path )) {
		mkdir($markup_path, 0775);
        }
        
        if (!file_put_contents($markup_path."/markup_".$markupCount.".txt")) {
        	$response->response = "Save Failed!";
        } else {
        	$response->response = "Saved!";
        }
	
	echo json_encode($response);
?>